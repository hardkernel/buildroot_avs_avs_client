/*
 * PortAudioMicrophoneWrapper.cpp
 *
 * Copyright (c) 2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "SampleApp/PortAudioMicrophoneWrapper.h"
#include "SampleApp/ConsolePrinter.h"
#include "AIP/AudioInputProcessor.h"
#include <AVSCommon/AVS/MessageRequest.h>
#include <AVSCommon/Utils/File/FileUtils.h>
#include <AVSCommon/Utils/JSON/JSONUtils.h>
#include <AVSCommon/Utils/Timing/TimeUtils.h>
#include <AVSCommon/Utils/Configuration/ConfigurationNode.h>

#include <alsa/asoundlib.h>

#include <gst/gst.h>
#include <glib.h>


namespace alexaClientSDK {
namespace sampleApp {

using avsCommon::avs::AudioInputStream;
using namespace capabilityAgents::aip;

#define REC_DEVICE_NAME "hw:0,3"
static const int NUM_INPUT_CHANNELS = 8;
static const int NUM_OUTPUT_CHANNELS = 0;
static const double SAMPLE_RATE = 48000;
static const unsigned long PREFERRED_SAMPLES_PER_CALLBACK_FOR_DSP = 768;
static const unsigned long PREFERRED_SAMPLES_PER_CALLBACK_FOR_ALSA = 256;
static const unsigned long RING_BUFFER_SIZE = PREFERRED_SAMPLES_PER_CALLBACK_FOR_DSP * NUM_INPUT_CHANNELS * 30;
static const std::string MUTE_CAPABILITY_AGENT_CONFIGURATION_ROOT_KEY = "alertsCapabilityAgent";
static const std::string MUTE_CAPABILITY_AGENT_AUDIO_ON_FILE_PATH_KEY = "muteOnSoundFilePath";
static const std::string MUTE_CAPABILITY_AGENT_AUDIO_OFF_FILE_PATH_KEY = "muteOffSoundFilePath";
int prev_state  = 0;
std::shared_ptr<alexaClientSDK::defaultClient::DefaultClient> PortAudioMicrophoneWrapper::m_client;
void (*PortAudioMicrophoneWrapper::call_notifier)(uint64_t startIndex , uint64_t endIndex);

#if DUMP_DATA
FILE* fp_input ; FILE* fp_output;
#endif

int loopCount = 0;

UINT32 inCount = 0;
UINT32 outCount = 0;
UINT32 in_samps = 0;
UINT32 in_chans = 0;
UINT32 out_samps = 0;
UINT32 out_chans = 0;

CAWELib *pAwelib;

std::vector<int16_t> audioData_SDS;

std::vector<int> out_samples;
std::vector<int> in_samples;
std::vector<int> ring_buffer;
volatile int rp = 0 ;
volatile int wp = 0;

PortAudioMicrophoneWrapper* wrapper;
#define SAMPLE_RATE 48000
#define WRITE_DEVICE_NAME "dmixer"
#define READ_FRAME 768
#define BUFFER_SIZE (SAMPLE_RATE/2)
#define PERIOD_SIZE (BUFFER_SIZE/4)

#define DSP_DEBUG 0
#define DSP_SOCKET 1

#if DSP_DEBUG == 1
#define WRITE_UNIT (READ_FRAME * 2)
#define RING_PCM_SIZE (BUFFER_SIZE*2)

int ring_pcm_buffer[RING_PCM_SIZE];
int write_pcm_buffer[WRITE_UNIT];
volatile int rp_pcm = 0 ;
volatile int wp_pcm = 0;
#endif

#if DSP_SOCKET == 1
/** The socket listener. */
static CTcpIO2 *ptcpIO = 0;
static UINT32 s_pktLen = 0;
static UINT32 s_partial = 0;
static UINT32 commandBuf[MAX_COMMAND_BUFFER_LEN];
/**
 * @brief Handle AWE server messages.
 * @param [in] pAwe				the library instance
 */
static void NotifyFunction(void *pAwe, int count)
{
    CAWELib *pAwelib = (CAWELib *)pAwe;
    SReceiveMessage *pCurrentReplyMsg = ptcpIO->BinaryGetMessage();
    const int msglen = pCurrentReplyMsg->size();

    if (msglen != 0) {
        // This is a binary message.
        UINT32 *txPacket_Host = (UINT32 *)(pCurrentReplyMsg->GetData());
        if (txPacket_Host) {
            UINT32 first = txPacket_Host[0];
            UINT32 len = (first >> 16);

            if (s_pktLen) {
                len = s_pktLen;
            }

            if (len > MAX_COMMAND_BUFFER_LEN) {
                printf("count=%d msglen=%d\n", count, msglen);
                printf("NotifyFunction: packet 0x%08x len %d too big (max %d)\n", first,
                       len, MAX_COMMAND_BUFFER_LEN);
                exit(1);
            }
            if (len * sizeof(int) > s_partial + msglen) {
                // Paxket is not complete, copy partial words.
                // printf("Partial message bytes=%d len=%d s_partial=%d\n",
                //	          msglen, len, s_partial);
                memcpy(((char *)commandBuf) + s_partial, txPacket_Host, msglen);
                s_partial += msglen;
                s_pktLen = len;
                return;
            }

            memcpy(((char *)commandBuf) + s_partial, txPacket_Host, msglen);
            s_partial = 0;
            s_pktLen = 0;

            // AWE processes the message.
            int ret = pAwelib->SendCommand(commandBuf, MAX_COMMAND_BUFFER_LEN);
            if (ret < 0) {
                printf("NotifyFunction: SendCommand failed with %d\n", ret);
                exit(1);
            }

            // Verify sane AWE reply.
            len = commandBuf[0] >> 16;
            if (len > MAX_COMMAND_BUFFER_LEN) {
                printf("NotifyFunction: reply packet len %d too big (max %d)\n",
                       len, MAX_COMMAND_BUFFER_LEN);
                exit(1);
            }

            ret = ptcpIO->SendBinaryMessage("", -1, (BYTE *)commandBuf, len * sizeof(UINT32));
            if (ret < 0) {
                printf("NotifyFunction: SendBinaryMessage failed with %d\n", ret);
                exit(1);
            }
        } else {
            printf("NotifyFunction: impossible no message pointer\n");
            exit(1);
        }
    } else {
        printf("NotifyFunction: illegal zero klength message\n");
        exit(1);
    }
}
#endif

#if DSP_DEBUG == 1
void fill_write_buffer(int* input)
{
    int space = rp_pcm - wp_pcm;
    if (space <= 0 ) space = RING_PCM_SIZE + space;
    if (space < READ_FRAME*2 ) {
        printf(" OVERFLOW IN fill_write_buffer wp_pcm %d , rp_pcm %d \n" , wp_pcm , rp_pcm);
    }

    for (int i = 0 ; i < READ_FRAME*2 ;  i = i+2) {
        ring_pcm_buffer[wp_pcm] = input[1];
        wp_pcm++;
        if (wp_pcm == RING_PCM_SIZE)
            wp_pcm = 0;
        ring_pcm_buffer[wp_pcm] = 0;
        wp_pcm++;
        if (wp_pcm == RING_PCM_SIZE)
            wp_pcm = 0;

        input += 2;
    }
}

void fill_write_buffer1(int* input)
{
    int space = rp_pcm - wp_pcm;
    if (space <= 0 ) space = RING_PCM_SIZE + space;

    if (space < READ_FRAME*2 ) {
        printf(" OVERFLOW IN fill_write_buffer1 wp_pcm %d , rp_pcm %d \n" , wp_pcm , rp_pcm);
    }

    for (int i = 0 ; i < READ_FRAME*8 ;  i = i + 8) {
        ring_pcm_buffer[wp_pcm] = input[0];
        wp_pcm++;

        if (wp_pcm == RING_PCM_SIZE)
            wp_pcm = 0;
        ring_pcm_buffer[wp_pcm] = input[1];
        wp_pcm++;
        if (wp_pcm == RING_PCM_SIZE)
            wp_pcm = 0;

        input = input + 8;
    }
}

void PortAudioMicrophoneWrapper::do_debug_pcm_write()
{
    printf("inside thread do_debug_pcm_write \n");
    snd_pcm_hw_params_t *params;
    snd_pcm_t *handle;
    int err;
    int dir;
    unsigned int sampleRate = SAMPLE_RATE;
    snd_pcm_uframes_t periodSize = PERIOD_SIZE;
    snd_pcm_uframes_t bufferSize = BUFFER_SIZE;
    int level;
    int first =0;

    err = snd_pcm_open(&handle, WRITE_DEVICE_NAME, SND_PCM_STREAM_PLAYBACK, 0);
    if (err) {
        printf( "Unable to open PCM device: \n");
        goto error;
    }

    snd_pcm_hw_params_alloca(&params);

    snd_pcm_hw_params_any(handle, params);

    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err) {
        printf("Error setting interleaved mode\n");
        goto error;
    }

    err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE);
    if (err) {
        printf("Error setting format: %s\n", snd_strerror(err));
        goto error;
    }

    err = snd_pcm_hw_params_set_channels(handle, params, 2);
    if (err) {
        printf( "Error setting channels: %s\n", snd_strerror(err));
        goto error;
    }

    err = snd_pcm_hw_params_set_buffer_size_near(handle, params, &bufferSize);
    if (err) {
        printf("Error setting buffer size (%ld): %s\n", bufferSize, snd_strerror(err));
        goto error;
    }

    err = snd_pcm_hw_params_set_period_size_near(handle, params, &periodSize, 0);
    if (err) {
        printf("Error setting period time (%ld): %s\n", periodSize, snd_strerror(err));
        goto error;
    }

    err = snd_pcm_hw_params_set_rate_near(handle, params, &sampleRate, &dir);
    if (err) {
        printf("Error setting sampling rate (%d): %s\n", sampleRate, snd_strerror(err));
        goto error;
    }

    snd_pcm_uframes_t final_buffer;
    err = snd_pcm_hw_params_get_buffer_size(params, &final_buffer);
    printf(" final buffer size %ld \n" , final_buffer);

    snd_pcm_uframes_t final_period;
    err = snd_pcm_hw_params_get_period_size(params, &final_period, &dir);
    printf(" final period size %ld \n" , final_period);

    /* Write the parameters to the driver */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        printf( "Unable to set HW parameters: %s\n", snd_strerror(err));
        goto error;
    }

    printf(" open write device is successful \n");

    while (1) {
        level = wp_pcm - rp_pcm;
        if (level < 0 ) level = RING_PCM_SIZE + level;

        if (first == 0 && level < RING_PCM_SIZE*3/4 ) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        } else
            first = 1;

        if (level >= WRITE_UNIT) {
            for (int i = 0 ; i < WRITE_UNIT ; i++) {
                write_pcm_buffer[i] = ring_pcm_buffer[rp_pcm];
                rp_pcm++;
                if (rp_pcm == RING_PCM_SIZE)
                    rp_pcm = 0;
            }

            err = snd_pcm_writei(handle, &write_pcm_buffer[0], WRITE_UNIT/2);
            if (err == -EPIPE)
                printf( "Underrun occurred from write: %d\n", err);
            if (err < 0) {
                err = snd_pcm_recover(handle, err, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                // Still an error, need to exit.
                if (err < 0) {
                    printf( "Error occured while writing: %s\n", snd_strerror(err));
                    goto error;
                }
            }
        }
    }

error:
    if (handle) snd_pcm_close(handle);
}
#endif

void setup_DSP() {
    UINT32 wireId1 = 1;
    UINT32 wireId2 = 2;
    UINT32 layoutId = 0;
    int error = 0;

    // Create AWE.
    pAwelib = AWELibraryFactory();
    error = pAwelib->Create("libtest", 1e9f, 1e7f);
    if (error < 0) {
        printf("[DSP] Create AWE failed with %d\n", error);
        delete pAwelib;
        exit(1);
    }
    printf("[DSP Using library '%s'\n", pAwelib->GetLibraryVersion());

    const char* file = "AMLogic_VUI_Solution_Model.awb";
    // Only load a layout if asked.
    error = pAwelib->LoadAwbFile(file);
    if (error < 0) {
        printf("[DSP] LoadAwbFile failed with %d(%s)\n", error, file);
        delete pAwelib;
        exit(1);
    }

    UINT32 in_complex = 0;
    UINT32 out_complex = 0;
    error = pAwelib->PinProps(in_samps, in_chans, in_complex, wireId1,
    out_samps, out_chans, out_complex, wireId2);

    if (error < 0) {
        printf("PinProps failed with %d\n", error);
        delete pAwelib;
        exit(1);
    }

    if (error > 0) {
        // We have a layout.
        if (in_complex) {
            in_chans *= 2;
        }

        if (out_complex) {
            out_chans *= 2;
        }
        printf("[DSP] AWB layout In: %d samples, %d chans ID=%d; out: %d samples, %d chans ID=%d\n",
             in_samps, in_chans, wireId1, out_samps, out_chans, wireId2);
        inCount = in_samps * in_chans;
        outCount = out_samps * out_chans;
    } else {
        // We have no layout.
        printf("[DSP] AWB No layout\n");
    }

    // Now we know the IDs, set them.
    pAwelib->SetLayoutAddresses(wireId1, wireId2, layoutId);

    in_samples.resize(inCount);
    out_samples.resize(outCount);
    ring_buffer.resize(RING_BUFFER_SIZE);

#if DSP_SOCKET == 1
    // Listen for AWE server on 15002.
    UINT32 listenPort = 15002;
    ptcpIO = new CTcpIO2();
    ptcpIO->SetNotifyFunction(pAwelib, NotifyFunction);
    HRESULT hr = ptcpIO->CreateBinaryListenerSocket(listenPort);
    if (FAILED(hr)) {
        printf("Could not open socket on %d failed with %d\n",
               listenPort, hr);
        delete ptcpIO;
        delete pAwelib;
        exit(1);
    }
#endif
}

std::unique_ptr<PortAudioMicrophoneWrapper> PortAudioMicrophoneWrapper::create(
        std::shared_ptr<AudioInputStream> stream , void(*fp)(uint64_t startIndex , uint64_t endIndex),
        std::shared_ptr<alexaClientSDK::defaultClient::DefaultClient> client) {

    if (!stream) {
        ConsolePrinter::simplePrint("Invalid stream passed to PortAudioMicrophoneWrapper");
        return nullptr;
    }
    std::unique_ptr<PortAudioMicrophoneWrapper> portAudioMicrophoneWrapper(
        new PortAudioMicrophoneWrapper(stream)
    );

    call_notifier = fp;
    m_client = client;

    if (!portAudioMicrophoneWrapper->initialize()) {
        ConsolePrinter::simplePrint("Failed to initialize PortAudioMicrophoneWrapper");
        return nullptr;
    }

#if DUMP_DATA
    fp_input = fopen("input.bin" , "wb");
    fp_output = fopen("output.bin" , "wb");
#endif
    portAudioMicrophoneWrapper->key_mute_sound_init();

    return portAudioMicrophoneWrapper;
}


static gboolean mute_play_state(GstBus * bus, GstMessage * msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *) data;

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
        //g_print("End of mute stream\n");
        g_main_loop_quit(loop);
    break;
    case GST_MESSAGE_ERROR:
        {
            gchar *debug;
            GError *error;
            gst_message_parse_error(msg, &error, &debug);
            g_free(debug);
            g_printerr("ERROR:%s\n", error->message);
            g_error_free(error);
            g_main_loop_quit(loop);
        }
    break;
    default:
    break;
    }
    return TRUE;
}

void PortAudioMicrophoneWrapper::key_mute_sound_thread() {
    GMainLoop *loop;
    GstElement *pipeline, *source, *decoder, *sink;
    GstBus *bus;
    const char* audioPath;
    gst_init(NULL, NULL);
    loop = g_main_loop_new(NULL, FALSE);
    printf("key_mute_sound_thread.....start!\n");
    while (1) {
       if (true == mute_sound_flag) {
           if (mic_mute_flag)
               audioPath = muteOnAudioFilePath.c_str();
           else
               audioPath = muteOffAudioFilePath.c_str();

    //printf("@@@@audioPath:%s\n",audioPath);

    pipeline = gst_pipeline_new("audio-player");
    source = gst_element_factory_make("filesrc", "file-source");
    decoder = gst_element_factory_make("mad", "mad-decoder");
    sink = gst_element_factory_make("autoaudiosink", "audio-output");

    if (!pipeline || !source || !decoder || !sink) {
        g_printerr("One element could not be created.Exiting.\n");
        return;
    }

    g_object_set(G_OBJECT(source), "location", audioPath, NULL);
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, mute_play_state, loop);
    gst_object_unref(bus);
    gst_bin_add_many(GST_BIN(pipeline), source, decoder, sink, NULL);
    gst_element_link_many(source, decoder, sink, NULL);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    //g_print("Running\n");
    g_main_loop_run(loop);
    //g_print("Returned,stopping playback\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline));
    mute_sound_flag=false;
    } else usleep(1000*500);
    }
    printf("key_mute_sound_threadi exit.....exit!\n");
}

bool PortAudioMicrophoneWrapper::key_mute_sound_init() {
    GMainLoop *loop;
    GstElement *pipeline, *source, *decoder, *sink;
    GstBus *bus;
    int par_num = 2;

    auto configurationRoot = avsCommon::utils::configuration::ConfigurationNode::getRoot()[MUTE_CAPABILITY_AGENT_CONFIGURATION_ROOT_KEY];
    if (!configurationRoot) {
        printf("could not load AlertsCapabilityAgent configuration root.\n");
        return false;
    }

    if (!configurationRoot.getString(MUTE_CAPABILITY_AGENT_AUDIO_ON_FILE_PATH_KEY, &muteOnAudioFilePath) ||
        muteOnAudioFilePath.empty()) {
        printf("could not read mute On audio file path.\n");
        return false;
    }
    if (!avsCommon::utils::file::fileExists(muteOnAudioFilePath)) {
        printf("could not open mute on audio file.");
        return false;
    }

    if (!configurationRoot.getString(MUTE_CAPABILITY_AGENT_AUDIO_OFF_FILE_PATH_KEY, &muteOffAudioFilePath) ||
        muteOffAudioFilePath.empty()) {
        printf("could not read mute Off audio file path.\n");
        return false;
    }
    if (!avsCommon::utils::file::fileExists(muteOffAudioFilePath)) {
        printf("could not open mute off audio file.");
        return false;
    }

    mute_sound_flag = false;

    p_mute_thread = std::thread (&PortAudioMicrophoneWrapper::key_mute_sound_thread,this);

    return true;
}

PortAudioMicrophoneWrapper::PortAudioMicrophoneWrapper(std::shared_ptr<AudioInputStream> stream):
        m_audioInputStream{stream}, m_paStream{nullptr}, mic_mute_flag{true} {

#if DSP_DEBUG == 1
    debug_pcm_write = std::thread(&PortAudioMicrophoneWrapper::do_debug_pcm_write ,this);
#endif

}

PortAudioMicrophoneWrapper::~PortAudioMicrophoneWrapper() {
    Pa_StopStream(m_paStream);
    Pa_CloseStream(m_paStream);
    Pa_Terminate();
}

bool PortAudioMicrophoneWrapper::initialize() {

    setup_DSP();

    m_writer = m_audioInputStream->createWriter(AudioInputStream::Writer::Policy::NONBLOCKABLE);
    if (!m_writer) {
        ConsolePrinter::simplePrint("Failed to create stream writer");
        return false;
    }
#if 0
    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) {
        ConsolePrinter::simplePrint("Failed to initialize PortAudio");
        return false;
    }
    err = Pa_OpenDefaultStream(
        &m_paStream,
        NUM_INPUT_CHANNELS,
        NUM_OUTPUT_CHANNELS,
        paInt32,
        SAMPLE_RATE,
        PREFERRED_SAMPLES_PER_CALLBACK_FOR_ALSA,
        PortAudioCallback,
        this
    );
#endif
    audioData_SDS.resize(PREFERRED_SAMPLES_PER_CALLBACK_FOR_ALSA);

#if 0
    if (err != paNoError) {
        ConsolePrinter::simplePrint("Failed to open PortAudio default stream");
        printf(" Failed to open PortAudio default stream \n");
        printf(" err %s \n" , Pa_GetErrorText(err));
        return false;
    }
#endif

    pcm_read_thread = std::thread (&PortAudioMicrophoneWrapper::do_pcm_read ,this);
    return true;
}

bool PortAudioMicrophoneWrapper::startStreamingMicrophoneData() {
    printf("startStreamingMicrophoneData\n");
    mic_mute_flag = true;
    mute_sound_flag = true;
#if 0
    std::lock_guard<std::mutex> lock{m_mutex};
    PaError err = Pa_StartStream(m_paStream);
    if (err != paNoError) {
        ConsolePrinter::simplePrint("Failed to start PortAudio stream");
        return false;
    }
#endif
    return true;
}

bool PortAudioMicrophoneWrapper::stopStreamingMicrophoneData() {
    printf("stopStreamingMicrophoneData\n");
    mic_mute_flag = false;
    mute_sound_flag = true;
#if 0
    std::lock_guard<std::mutex> lock{m_mutex};
    PaError err = Pa_StopStream(m_paStream);
    if (err != paNoError) {
        ConsolePrinter::simplePrint("Failed to stop PortAudio stream");
        return false;
    }
#endif
    return true;
}

//in : 32bit , 48K , 1 ch (outsamples from DSP )
//out : 16bit , 16K , 1 ch ( every 3rd sample ) ( audioData_SDS )
void convert_DSP(int* input , short *output , int frame_num)
{
    for (int i = 0 ; i < frame_num  ; i += 3) {
        *output = *input >> 14;
        output++;
        input += 3;
    }
}

//in : 32bit , 48K , 2 ch (outsamples from DSP )
//out : 16bit , 16K , 1 ch ( every 6th sample ) ( audioData_SDS )
void convert_DSP_2ch(int* input , short *output , int frame_num)
{
    for (int i = 0 ; i < frame_num*2  ; i += 6) {
        *output = *input >> 16;
        output++;
        input += 6;
    }
}

// in : 32bit , 48K , 8 channels
// out : 16bit , 48K , 1 channel
int convert(int *input, short *output, int frame_num)
{
    int i;
    for (i = 0; i < frame_num; i++) {
        output[i] = input[8*i]   >> 14;
    }
    return 0;
}

// input : 8 ch , 0,1 feedback , 2,3,4,5 are data , 6,7 is empty , 32 bit
// output : 10 ch , 0,1,2,3,4,5 : data , 6,7 not used , 8,9 : feedback , 32 bit
int convert1(int *input, int *output, int frame_num)
{
    int i;
    memset(output , 0 , frame_num*10);
    for (i = 0; i < frame_num*8; i = i+8) {
        output[0] = input [0];
        output[1] = input [1];
        output[2] = input [2];
        output[3] = input [3];
        output[4] = input [4];
        output[5] = input [5];
        output[8] = input [6];
        output[9] = input [7];

        input += 8;
        output += 10;
    }

    return 0;
}

void do_dsp_processing_fn(int* in_samples , int* out_samples , int inCount , int outCount , int& result , Sample* startIndex,
                             Sample* endIndex ) {

    int error = 0;
    Sample retVal;

    //printf(" do_dsp_processing_fn \n");
#if 0
    memset(&in_samples[0], 0, inCount * sizeof(short));

    //convert to 10 channels , 32 bit input
    for (UINT32 i = 0 ; i < PREFERRED_SAMPLES_PER_CALLBACK_FOR_DSP; i++) {
        in_samples[i*10] = record_bufffer[i] << 16;
    }
#endif
    loopCount += PREFERRED_SAMPLES_PER_CALLBACK_FOR_DSP;
    if (loopCount > 16000 * 8) {
        printf("detection loop heart beat ...\n");
        loopCount = 0;
    }

    if (pAwelib->IsStarted()) {
        error = pAwelib->PumpAudio(&in_samples[0], &out_samples[0], inCount, outCount);
    } else {
        memset(&out_samples[0] , 0 , outCount*4);
    }

#if DUMP_DATA
    //printf(" post PumpAudio \n");
    fwrite(&in_samples[0] , PREFERRED_SAMPLES_PER_CALLBACK_FOR_DSP*10 , sizeof(int) , fp_input);
    fflush(fp_input);
#endif

#if DSP_DEBUG == 1
    fill_write_buffer(&out_samples[0]);
#endif
    if (error < 0) {
        printf("[DSP pump of audio failed with %d.\n", error);
        delete pAwelib;
        exit(1);
    }

    error = pAwelib->FetchValues(MAKE_ADDRESS(AWE_isTriggered_ID, AWE_isTriggered_value_OFFSET), 1,
                        &retVal,0);
    result = retVal.iVal;

    error = pAwelib->FetchValues(MAKE_ADDRESS(AWE_startIndex_ID, AWE_startIndex_value_OFFSET), 1,
                        startIndex , 0);
    //printf(" error startindex %d \n" , error);
    error = pAwelib->FetchValues(MAKE_ADDRESS(AWE_endIndex_ID, AWE_endIndex_value_OFFSET), 1,
                        endIndex , 0);

    //printf(" error endindex %d \n" , error);
    //printf("[DSP] detectionResult %d \n" , result);
}

void PortAudioMicrophoneWrapper::do_dsp_processing() {
    int result;
    Sample DSP_state;
    Sample startIndex[AWE_startIndex_value_SIZE];
    Sample endIndex[AWE_endIndex_value_SIZE];
    uint64_t total_samps = 0;
    wrapper = static_cast<PortAudioMicrophoneWrapper*>(this);
    int detection_count = 0;

    while (1) {

        int level = wp - rp;
        if (level < 0 ) level = RING_BUFFER_SIZE + level;
        //printf(" do_dsp_processing level %d \n" , level );
        if (level >= (int)PREFERRED_SAMPLES_PER_CALLBACK_FOR_DSP*NUM_INPUT_CHANNELS) {

            convert1(&ring_buffer[rp] , &in_samples[0], PREFERRED_SAMPLES_PER_CALLBACK_FOR_DSP);
            //fill_write_buffer1(&ring_buffer[rp]);

            AudioInputProcessor::ObserverInterface::State state = m_client->m_audioInputProcessor->getState();
            switch (state) {
                case AudioInputProcessor::ObserverInterface::State::IDLE:
                    DSP_state.iVal = 0;
                    break;
                case AudioInputProcessor::ObserverInterface::State::EXPECTING_SPEECH:
                    DSP_state.iVal = 1;
                    break;
                case AudioInputProcessor::ObserverInterface::State::RECOGNIZING:
                    DSP_state.iVal = 2;
                    break;
                case AudioInputProcessor::ObserverInterface::State::BUSY:
                    DSP_state.iVal = 3;
            }

            if (DSP_state.iVal != prev_state) {
                int ret = pAwelib->SetValues(MAKE_ADDRESS(AWE_VR_State_ID, AWE_VR_State_value_OFFSET ), 1,&DSP_state,0);
                if (ret != 0)
                    printf(" Error from SetValues %d " , ret);

                    //printf(" Switch from state %d to state %d \n" , prev_state , DSP_state.iVal);
                    prev_state = DSP_state.iVal;
            }

            do_dsp_processing_fn(&in_samples[0],&out_samples[0],inCount,outCount, result , &startIndex[0] , &endIndex[0] );

            convert_DSP_2ch(&out_samples[0], &audioData_SDS[0], PREFERRED_SAMPLES_PER_CALLBACK_FOR_DSP);

#if DUMP_DATA
            fwrite(&audioData_SDS[0] , PREFERRED_SAMPLES_PER_CALLBACK_FOR_ALSA*1 , sizeof(short) , fp_output);
            fflush(fp_output);
#endif
            ssize_t returnCode = wrapper->m_writer->write(audioData_SDS.data(), audioData_SDS.size());
            total_samps += audioData_SDS.size();

            if (returnCode <= 0) {
                ConsolePrinter::simplePrint("Failed to write to stream.");
                exit(1);
            }

            if (result == 1) {
                detection_count++;
                printf(" wake word detected %d \n" , detection_count);
                call_notifier(total_samps - startIndex[0].iVal , total_samps - endIndex[0].iVal);
            }

            rp = rp+ (int)PREFERRED_SAMPLES_PER_CALLBACK_FOR_DSP*NUM_INPUT_CHANNELS;
            if (rp > (int)RING_BUFFER_SIZE) {
                printf(" rp out of bound \n");
            }

            if (rp == RING_BUFFER_SIZE)
                rp = 0;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
}

int PortAudioMicrophoneWrapper::PortAudioCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long numSamples,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {

    wrapper = static_cast<PortAudioMicrophoneWrapper*>(userData);

    if (wp < rp && wp + (int)PREFERRED_SAMPLES_PER_CALLBACK_FOR_ALSA*NUM_INPUT_CHANNELS >= rp ) {
        printf(" OVERFLOW \n");
    }
    printf(" coming to PortAudioCallback \n");
    memcpy(&ring_buffer[wp] , (int*)inputBuffer , PREFERRED_SAMPLES_PER_CALLBACK_FOR_ALSA*4*NUM_INPUT_CHANNELS);

    wp = wp + PREFERRED_SAMPLES_PER_CALLBACK_FOR_ALSA*NUM_INPUT_CHANNELS;

    if (wp > (int)RING_BUFFER_SIZE)
        printf(" wp out of bounds \n");

    if (wp == RING_BUFFER_SIZE)
        wp = 0;

    return paContinue;
}

void PortAudioMicrophoneWrapper::do_pcm_read() {
    printf(" do pcm read \n");

    dsp_process = std::thread (&PortAudioMicrophoneWrapper::do_dsp_processing ,this);

    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    int err;
    int dir;
    int buffer[READ_FRAME * 8 * 4];
    unsigned int sampleRate = SAMPLE_RATE;
    snd_pcm_uframes_t periodSize = PERIOD_SIZE;
    snd_pcm_uframes_t bufferSize = BUFFER_SIZE;

    err = snd_pcm_open(&handle, REC_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0);
    if (err) {
        printf( "Unable to open PCM device: \n");
        goto error;
    }

    snd_pcm_hw_params_alloca(&params);

    snd_pcm_hw_params_any(handle, params);

    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err) {
        printf("Error setting interleaved mode\n");
        goto error;
    }

    err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE);
    if (err) {
        printf("Error setting format: %s\n", snd_strerror(err));
        goto error;
    }

    err = snd_pcm_hw_params_set_channels(handle, params, 8);
    if (err) {
        printf( "Error setting channels: %s\n", snd_strerror(err));
        goto error;
    }

    err = snd_pcm_hw_params_set_buffer_size_near(handle, params, &bufferSize);
    if (err) {
        printf("Error setting buffer size (%ld): %s\n", bufferSize, snd_strerror(err));
        goto error;
    }

    err = snd_pcm_hw_params_set_period_size_near(handle, params, &periodSize, 0);
    if (err) {
        printf("Error setting period time (%ld): %s\n", periodSize, snd_strerror(err));
        goto error;
    }

    err = snd_pcm_hw_params_set_rate_near(handle, params, &sampleRate, &dir);
    if (err) {
        printf("Error setting sampling rate (%d): %s\n", sampleRate, snd_strerror(err));
        goto error;
    }

    snd_pcm_uframes_t final_buffer;
    err = snd_pcm_hw_params_get_buffer_size(params, &final_buffer);
    printf(" final buffer size %ld \n" , final_buffer);

    snd_pcm_uframes_t final_period;
    err = snd_pcm_hw_params_get_period_size(params, &final_period, &dir);
    printf(" final period size %ld \n" , final_period);

    /* Write the parameters to the driver */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        printf( "Unable to set HW parameters: %s\n", snd_strerror(err));
        goto error;
    }

    printf(" ready to read \n" );

    while (1) {
        err = snd_pcm_readi(handle, buffer, READ_FRAME);
        if (err == -EPIPE) printf( "Overrun occurred: %d\n", err);

        if (err < 0) {
            err = snd_pcm_recover(handle, err, 0);
            // Still an error, need to exit.
            if (err < 0)
            {
                printf( "Error occured while recording: %s\n", snd_strerror(err));
                goto error;
            }
        }

        int space = wp - rp;
        if (space <= 0 ) space = space + RING_BUFFER_SIZE;
        if (space < (int)READ_FRAME*NUM_INPUT_CHANNELS) {
            printf(" OVERFLOW \n");
        }

        if (mic_mute_flag) {
            memcpy(&ring_buffer[wp] , (int*)buffer , READ_FRAME*4*NUM_INPUT_CHANNELS);
            wp = wp + READ_FRAME*NUM_INPUT_CHANNELS;
        }

        if (wp > (int)RING_BUFFER_SIZE)
            printf(" wp out of bounds \n");

        if (wp == RING_BUFFER_SIZE)
            wp = 0;
    }

error:
    if (handle) snd_pcm_close(handle);
}
} // namespace sampleApp
} // namespace alexaClientSDK
