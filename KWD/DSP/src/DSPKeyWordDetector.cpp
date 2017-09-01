
#include <AVSCommon/Utils/Logger/Logger.h>
#include <AVSCommon/Utils/Memory/Memory.h>

#include "DSPKeyWordDetector.h"

#include <memory>
#include <sstream>

#include <cassert>


namespace alexaClientSDK {
namespace kwd {

using namespace avsCommon;
using namespace avsCommon::avs;
using namespace avsCommon::sdkInterfaces;
using namespace avsCommon::utils;

/// The number of hertz per kilohertz.
static const size_t HERTZ_PER_KILOHERTZ = 1000;

/// The timeout to use for read calls to the SharedDataStream.
const std::chrono::milliseconds TIMEOUT_FOR_READ_CALLS = std::chrono::milliseconds(10000);


std::unique_ptr<DSPKeyWordDetector> DSPKeyWordDetector::create(
            std::shared_ptr<AudioInputStream> stream,
            AudioFormat audioFormat,
            std::unordered_set<std::shared_ptr<KeyWordObserverInterface>> keyWordObservers,
            std::unordered_set<std::shared_ptr<KeyWordDetectorStateObserverInterface>>
                keyWordDetectorStateObservers,
            const std::string& resourceFilePath,
            const std::vector<DSPConfiguration> dspConfigurations,
            float audioGain,
            bool applyFrontEnd,
            std::chrono::milliseconds msToPushPerIteration) {
    if (!stream) {
        printf("DSP key word detector must be initialized with a valid stream \n");
        return nullptr;
    }
    // TODO: ACSDK-249 - Investigate cpu usage of converting bytes between endianness and if it's not too much, do it.
    if (isByteswappingRequired(audioFormat)) {
        printf("Audio data endianness must match system endianness \n");
        return nullptr;
    }
    std::unique_ptr<DSPKeyWordDetector> detector(
        new DSPKeyWordDetector(
            stream,
            audioFormat,
            keyWordObservers,
            keyWordDetectorStateObservers,
            resourceFilePath,
            dspConfigurations,
            audioGain,
            applyFrontEnd,
            msToPushPerIteration
        )
    );

    if (!detector->init(audioFormat)) {
        printf("Unable to initialize Kitt.ai detector \n");
        return nullptr;
    }

    return detector;
}

DSPKeyWordDetector::~DSPKeyWordDetector() {
    m_isShuttingDown = true;

    m_streamReader->close();

    if (m_detectionThread.joinable()) {
        m_detectionThread.join();
    }
}

DSPKeyWordDetector::DSPKeyWordDetector(
        std::shared_ptr<AudioInputStream> stream,
        avsCommon::utils::AudioFormat audioFormat,
        std::unordered_set<std::shared_ptr<KeyWordObserverInterface>> keyWordObservers,
        std::unordered_set<std::shared_ptr<KeyWordDetectorStateObserverInterface>>
            keyWordDetectorStateObservers,
        const std::string& resourceFilePath,
        const std::vector<DSPConfiguration> dspConfigurations,
        float audioGain,
        bool applyFrontEnd,
        std::chrono::milliseconds msToPushPerIteration):
    AbstractKeywordDetector(keyWordObservers, keyWordDetectorStateObservers),
    m_stream{stream},
    m_maxSamplesPerPush{(audioFormat.sampleRateHz / HERTZ_PER_KILOHERTZ) * msToPushPerIteration.count()} {

    std::stringstream sensitivities;
    std::stringstream modelPaths;
#if 0
        UINT32 wireId1 = 1;
	UINT32 wireId2 = 2;
	UINT32 layoutId = 0;
	int error = 0;
	// Create AWE.
	pAwelib = AWELibraryFactory();
	error = pAwelib->Create("libtest", 1e9f, 1e7f);
	if (error < 0)
	{
		printf("[DSP] Create AWE failed with %d\n", error);
		delete pAwelib;
		exit(1);
	}
	printf("[DSP Using library '%s'\n", pAwelib->GetLibraryVersion());

	if (1/*dspConfigurations[0].modelFilePath*/)
	{
		const char* file = "AMLogic_VUI_Solution_v5_VoiceOnly_release.awb";
		// Only load a layout if asked.
		error = pAwelib->LoadAwbFile(file);
		if (error < 0)
		{
			printf("[DSP] LoadAwbFile failed with %d\n", error);
			delete pAwelib;
			exit(1);
		}
	}

	{
		UINT32 in_complex = 0;
		UINT32 out_complex = 0;
		error = pAwelib->PinProps(in_samps, in_chans, in_complex, wireId1,
			out_samps, out_chans, out_complex, wireId2);
		if (error < 0)
		{
			printf("PinProps failed with %d\n", error);
			delete pAwelib;
			exit(1);
		}
		if (error > 0)
		{
			// We have a layout.
			if (in_complex)
			{
				in_chans *= 2;
			}
			if (out_complex)
			{
				out_chans *= 2;
			}
			printf("[DSP] AWB layout In: %d samples, %d chans ID=%d; out: %d samples, %d chans ID=%d\n",
				in_samps, in_chans, wireId1, out_samps, out_chans, wireId2);
			inCount = in_samps * in_chans;
			outCount = out_samps * out_chans;
		}
		else
		{
			// We have no layout.
			printf("[DSP] AWB No layout\n");
		}
	}

	// Now we know the IDs, set them.
	pAwelib->SetLayoutAddresses(wireId1, wireId2, layoutId);
#endif
}

bool DSPKeyWordDetector::init(avsCommon::utils::AudioFormat audioFormat) {
    /*
    if (!isAudioFormatCompatibleWithKittAi(audioFormat)) {
        return false;
    }
    */

    m_streamReader = m_stream->createReader(AudioInputStream::Reader::Policy::BLOCKING);
    if (!m_streamReader) {
        printf("[DSP] Unable to create Stream reader\n");
        return false;
    }
    m_isShuttingDown = false;
    //m_detectionThread = std::thread(&DSPKeyWordDetector::detectionLoop, this);
    return true;
}

void DSPKeyWordDetector::notifyDetection(int startIndex , int endIndex) {

    printf(" DSPKeyword startIndex %d endIndex %d \n" , startIndex ,endIndex);
    notifyKeyWordDetectorStateObservers(KeyWordDetectorStateObserverInterface::KeyWordDetectorState::ACTIVE);

    notifyKeyWordObservers(m_stream,
                    "ALEXA",
                    startIndex , //UNSPECIFIED_INDEX,
                    endIndex ); //UNSPECIFIED_INDEX);
}

void DSPKeyWordDetector::detectionLoop() {
#if 0
    notifyKeyWordDetectorStateObservers(KeyWordDetectorStateObserverInterface::KeyWordDetectorState::ACTIVE);
    while (!m_isShuttingDown) {

        int detectionResult;
        detectionResult = fun();

	//printf("[DSP] detectionResult %d \n" , detectionResult);

        if (detectionResult == 1) {
            printf("[DSP] wake word detected\n");
            printf(" value of n %d \n" , n);
            n++;
            detectionResult = 0;

            notifyKeyWordObservers(m_stream,
                  "ALEXA",
                  KeyWordObserverInterface::UNSPECIFIED_INDEX,
                  KeyWordObserverInterface::UNSPECIFIED_INDEX);
        }
    }

    m_streamReader->close();
#endif

}

} // namespace kwd
} // namespace alexaClientSDK


