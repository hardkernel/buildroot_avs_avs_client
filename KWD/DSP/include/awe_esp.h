/****************************************************************************
 *
 *		Target Tuning Symbol File
 *		-------------------------
 *
 *          Generated on:  23-Mar-2018 19:04:44
 *
 ***************************************************************************/

#ifndef AMLOGIC_VUI_SOLUTION_V13A_VOICEONLY_GEN2_TUNINGGUI_ESPSIMPLE_H
#define AMLOGIC_VUI_SOLUTION_V13A_VOICEONLY_GEN2_TUNINGGUI_ESPSIMPLE_H

// ----------------------------------------------------------------------
// LatencyControlSamps [Delay]
// Time delay in which the delay is specified in samples

#define AWE_LatencyControlSamps_ID 30001

// int maxDelay - Maximum delay, in samples. The size of the delay
//         buffer is (maxDelay+1)*numChannels.
#define AWE_LatencyControlSamps_maxDelay_OFFSET 8
#define AWE_LatencyControlSamps_maxDelay_MASK 0x00000100
#define AWE_LatencyControlSamps_maxDelay_SIZE -1

// int currentDelay - Current delay.
// Default value: 0
// Range: 0 to 6400.  Step size = 1
#define AWE_LatencyControlSamps_currentDelay_OFFSET 9
#define AWE_LatencyControlSamps_currentDelay_MASK 0x00000200
#define AWE_LatencyControlSamps_currentDelay_SIZE -1

// int stateIndex - Index of the oldest state variable in the array of
//         state variables.
#define AWE_LatencyControlSamps_stateIndex_OFFSET 10
#define AWE_LatencyControlSamps_stateIndex_MASK 0x00000400
#define AWE_LatencyControlSamps_stateIndex_SIZE -1

// int stateHeap - Heap in which to allocate memory.
#define AWE_LatencyControlSamps_stateHeap_OFFSET 11
#define AWE_LatencyControlSamps_stateHeap_MASK 0x00000800
#define AWE_LatencyControlSamps_stateHeap_SIZE -1

// float state[6658] - State variable array.
#define AWE_LatencyControlSamps_state_OFFSET 12
#define AWE_LatencyControlSamps_state_MASK 0x00001000
#define AWE_LatencyControlSamps_state_SIZE 6658


// ----------------------------------------------------------------------
// TrimGains [ScalerNV2]
// General purpose scaler with separate gains per channel

#define AWE_MicTrimGains_ID 30015

// float masterGain - Overall gain to apply.
// Default value: 24
// Range: -24 to 24
#define AWE_MicTrimGains_masterGain_OFFSET 8
#define AWE_MicTrimGains_masterGain_MASK 0x00000100
#define AWE_MicTrimGains_masterGain_SIZE -1

// float smoothingTime - Time constant of the smoothing process (0 =
//         unsmoothed).
// Default value: 0
// Range: 0 to 1000
#define AWE_MicTrimGains_smoothingTime_OFFSET 9
#define AWE_MicTrimGains_smoothingTime_MASK 0x00000200
#define AWE_MicTrimGains_smoothingTime_SIZE -1

// int isDB - Selects between linear (=0) and dB (=1) operation
// Default value: 1
// Range: unrestricted
#define AWE_MicTrimGains_isDB_OFFSET 10
#define AWE_MicTrimGains_isDB_MASK 0x00000400
#define AWE_MicTrimGains_isDB_SIZE -1

// float smoothingCoeff - Smoothing coefficient.
#define AWE_MicTrimGains_smoothingCoeff_OFFSET 11
#define AWE_MicTrimGains_smoothingCoeff_MASK 0x00000800
#define AWE_MicTrimGains_smoothingCoeff_SIZE -1

// float trimGain[10] - Array of trim gains, one per channel
// Default value:
//     0
//     0
//     0
//     0
//     0
//     0
//     0
//     0
//     0
//     0
// Range: -24 to 24
#define AWE_MicTrimGains_trimGain_OFFSET 12
#define AWE_MicTrimGains_trimGain_MASK 0x00001000
#define AWE_MicTrimGains_trimGain_SIZE 10

// float targetGain[10] - Computed target gains in linear units
#define AWE_MicTrimGains_targetGain_OFFSET 13
#define AWE_MicTrimGains_targetGain_MASK 0x00002000
#define AWE_MicTrimGains_targetGain_SIZE 10

// float currentGain[10] - Instanteous gains.  These ramp towards
//         targetGain
#define AWE_MicTrimGains_currentGain_OFFSET 14
#define AWE_MicTrimGains_currentGain_MASK 0x00004000
#define AWE_MicTrimGains_currentGain_SIZE 10


// ----------------------------------------------------------------------
// InputMeter [Meter]
// Peak and RMS meter module

#define AWE_InputMeter_ID 30000

// int meterType - Operating mode of the meter. Selects between peak and
//         RMS calculations. See the discussion section for more details.
// Default value: 18
// Range: unrestricted
#define AWE_InputMeter_meterType_OFFSET 8
#define AWE_InputMeter_meterType_MASK 0x00000100
#define AWE_InputMeter_meterType_SIZE -1

// float attackTime - Attack time of the meter. Specifies how quickly
//         the meter value rises.
#define AWE_InputMeter_attackTime_OFFSET 9
#define AWE_InputMeter_attackTime_MASK 0x00000200
#define AWE_InputMeter_attackTime_SIZE -1

// float releaseTime - Release time of the meter. Specifies how quickly
//         the meter decays.
#define AWE_InputMeter_releaseTime_OFFSET 10
#define AWE_InputMeter_releaseTime_MASK 0x00000400
#define AWE_InputMeter_releaseTime_SIZE -1

// float attackCoeff - Internal coefficient that realizes the attack
//         time.
#define AWE_InputMeter_attackCoeff_OFFSET 11
#define AWE_InputMeter_attackCoeff_MASK 0x00000800
#define AWE_InputMeter_attackCoeff_SIZE -1

// float releaseCoeff - Internal coefficient that realizes the release
//         time.
#define AWE_InputMeter_releaseCoeff_OFFSET 12
#define AWE_InputMeter_releaseCoeff_MASK 0x00001000
#define AWE_InputMeter_releaseCoeff_SIZE -1

// float value[10] - Array of meter output values, one per channel.
#define AWE_InputMeter_value_OFFSET 13
#define AWE_InputMeter_value_MASK 0x00002000
#define AWE_InputMeter_value_SIZE 10


// ----------------------------------------------------------------------
// FreqDomainProcessing.NoiseReductionDB [Sink]
// Copies the data at the input pin and stores it in an internal buffer.

#define AWE_FreqDomainProcessing____NoiseReductionDB_ID 30010

// int enable - To enable or disable the plotting.
// Default value: 0
// Range: unrestricted
#define AWE_FreqDomainProcessing____NoiseReductionDB_enable_OFFSET 8
#define AWE_FreqDomainProcessing____NoiseReductionDB_enable_MASK 0x00000100
#define AWE_FreqDomainProcessing____NoiseReductionDB_enable_SIZE -1

// float value[1] - Captured values.
#define AWE_FreqDomainProcessing____NoiseReductionDB_value_OFFSET 9
#define AWE_FreqDomainProcessing____NoiseReductionDB_value_MASK 0x00000200
#define AWE_FreqDomainProcessing____NoiseReductionDB_value_SIZE 1

// float yRange[2] - Y-axis range.
// Default value:
//     -5  5
// Range: unrestricted
#define AWE_FreqDomainProcessing____NoiseReductionDB_yRange_OFFSET 10
#define AWE_FreqDomainProcessing____NoiseReductionDB_yRange_MASK 0x00000400
#define AWE_FreqDomainProcessing____NoiseReductionDB_yRange_SIZE 2


// ----------------------------------------------------------------------
// FreqDomainProcessing.Direction [SinkInt]
// Copies the data at the input pin and stores it in an internal buffer

#define AWE_FreqDomainProcessing____Direction_ID 30009

// int value[1] - Captured values
#define AWE_FreqDomainProcessing____Direction_value_OFFSET 8
#define AWE_FreqDomainProcessing____Direction_value_MASK 0x00000100
#define AWE_FreqDomainProcessing____Direction_value_SIZE 1


// ----------------------------------------------------------------------
// FreqDomainProcessing.AEC_Perf_Sink [Sink]
// Copies the data at the input pin and stores it in an internal buffer.

#define AWE_FreqDomainProcessing____AEC_Perf_Sink_ID 30014

// int enable - To enable or disable the plotting.
// Default value: 0
// Range: unrestricted
#define AWE_FreqDomainProcessing____AEC_Perf_Sink_enable_OFFSET 8
#define AWE_FreqDomainProcessing____AEC_Perf_Sink_enable_MASK 0x00000100
#define AWE_FreqDomainProcessing____AEC_Perf_Sink_enable_SIZE -1

// float value[1250] - Captured values.
#define AWE_FreqDomainProcessing____AEC_Perf_Sink_value_OFFSET 9
#define AWE_FreqDomainProcessing____AEC_Perf_Sink_value_MASK 0x00000200
#define AWE_FreqDomainProcessing____AEC_Perf_Sink_value_SIZE 1250

// float yRange[2] - Y-axis range.
// Default value:
//     -5  5
// Range: unrestricted
#define AWE_FreqDomainProcessing____AEC_Perf_Sink_yRange_OFFSET 10
#define AWE_FreqDomainProcessing____AEC_Perf_Sink_yRange_MASK 0x00000400
#define AWE_FreqDomainProcessing____AEC_Perf_Sink_yRange_SIZE 2


// ----------------------------------------------------------------------
// FreqDomainProcessing.SetDirection [SourceInt]
// Source buffer holding 1 wire of integer data

#define AWE_FreqDomainProcessing____SetDirection_ID 30008

// int value[1] - Array of interleaved audio data
// Default value:
//     5
// Range: unrestricted
#define AWE_FreqDomainProcessing____SetDirection_value_OFFSET 8
#define AWE_FreqDomainProcessing____SetDirection_value_MASK 0x00000100
#define AWE_FreqDomainProcessing____SetDirection_value_SIZE 1


// ----------------------------------------------------------------------
// FreqDomainProcessing.ManualDirection [Multiplexor]
// Selects one of N inputs

#define AWE_FreqDomainProcessing____ManualDirection_ID 30007

// int indexPinFlag - Specifies index pin available or not.
#define AWE_FreqDomainProcessing____ManualDirection_indexPinFlag_OFFSET 8
#define AWE_FreqDomainProcessing____ManualDirection_indexPinFlag_MASK 0x00000100
#define AWE_FreqDomainProcessing____ManualDirection_indexPinFlag_SIZE -1

// int index - Specifies which input pin to route to the output. The
//         index is zero based.
// Default value: 1
// Range: 0 to 1
#define AWE_FreqDomainProcessing____ManualDirection_index_OFFSET 9
#define AWE_FreqDomainProcessing____ManualDirection_index_MASK 0x00000200
#define AWE_FreqDomainProcessing____ManualDirection_index_SIZE -1


// ----------------------------------------------------------------------
// WakeWordProcessing.TriggerStorage [TriggeredSink]
// Copies the data at the input pin and stores it in an internal buffer.
// When triggered, buffer is frozen.

#define AWE_WakeWordProcessing____TriggerStorage_ID 31000

// int reset - Data will remain latched until reset set to 1.
#define AWE_WakeWordProcessing____TriggerStorage_reset_OFFSET 8
#define AWE_WakeWordProcessing____TriggerStorage_reset_MASK 0x00000100
#define AWE_WakeWordProcessing____TriggerStorage_reset_SIZE -1

// int ctrl_index - Index of first non-zero element of the ctrl signal.
//         Any non-zero element will trigger a data acquisition.
#define AWE_WakeWordProcessing____TriggerStorage_ctrl_index_OFFSET 9
#define AWE_WakeWordProcessing____TriggerStorage_ctrl_index_MASK 0x00000200
#define AWE_WakeWordProcessing____TriggerStorage_ctrl_index_SIZE -1

// int manual_trigger - Trigger data acquisition from Matlab
#define AWE_WakeWordProcessing____TriggerStorage_manual_trigger_OFFSET 10
#define AWE_WakeWordProcessing____TriggerStorage_manual_trigger_MASK 0x00000400
#define AWE_WakeWordProcessing____TriggerStorage_manual_trigger_SIZE -1

// float value[16000] - Captured values.
#define AWE_WakeWordProcessing____TriggerStorage_value_OFFSET 11
#define AWE_WakeWordProcessing____TriggerStorage_value_MASK 0x00000800
#define AWE_WakeWordProcessing____TriggerStorage_value_SIZE 16000


// ----------------------------------------------------------------------
// endIndex [SinkInt]
// Copies the data at the input pin and stores it in an internal buffer

#define AWE_endIndex_ID 30012

// int value[1] - Captured values
#define AWE_endIndex_value_OFFSET 8
#define AWE_endIndex_value_MASK 0x00000100
#define AWE_endIndex_value_SIZE 1


// ----------------------------------------------------------------------
// isTriggered [SinkInt]
// Copies the data at the input pin and stores it in an internal buffer

#define AWE_isTriggered_ID 30003

// int value[1] - Captured values
#define AWE_isTriggered_value_OFFSET 8
#define AWE_isTriggered_value_MASK 0x00000100
#define AWE_isTriggered_value_SIZE 1


// ----------------------------------------------------------------------
// numTriggers [RunningStatistics]
// Computes long term statistics

#define AWE_numTriggers_ID 30013

// int statisticsType - Type of statistics needed.
// Default value: 3
// Range: 0 to 8
#define AWE_numTriggers_statisticsType_OFFSET 8
#define AWE_numTriggers_statisticsType_MASK 0x00000100
#define AWE_numTriggers_statisticsType_SIZE -1

// float value - Instantaneous output value.
#define AWE_numTriggers_value_OFFSET 9
#define AWE_numTriggers_value_MASK 0x00000200
#define AWE_numTriggers_value_SIZE -1

// float mean - State variable for mean.
#define AWE_numTriggers_mean_OFFSET 10
#define AWE_numTriggers_mean_MASK 0x00000400
#define AWE_numTriggers_mean_SIZE -1

// float avgEnergy - State variable for average energy.
#define AWE_numTriggers_avgEnergy_OFFSET 11
#define AWE_numTriggers_avgEnergy_MASK 0x00000800
#define AWE_numTriggers_avgEnergy_SIZE -1

// int numBlocksProcessed - Counter for the number of blocks processed.
#define AWE_numTriggers_numBlocksProcessed_OFFSET 12
#define AWE_numTriggers_numBlocksProcessed_MASK 0x00001000
#define AWE_numTriggers_numBlocksProcessed_SIZE -1

// int reset - Used to identify the first block which is processed.
#define AWE_numTriggers_reset_OFFSET 13
#define AWE_numTriggers_reset_MASK 0x00002000
#define AWE_numTriggers_reset_SIZE -1


// ----------------------------------------------------------------------
// startIndex [SinkInt]
// Copies the data at the input pin and stores it in an internal buffer

#define AWE_startIndex_ID 30011

// int value[1] - Captured values
#define AWE_startIndex_value_OFFSET 8
#define AWE_startIndex_value_MASK 0x00000100
#define AWE_startIndex_value_SIZE 1


// ----------------------------------------------------------------------
// AmazonESP1 [AmazonESP]
// Amazon ESP

#define AWE_AVS_ESP_ID 30020

// fract32 lastVoiceEnergy - VoiceEnergy result from last calculation
#define AWE_AVS_ESP_lastVoiceEnergy_OFFSET 8
#define AWE_AVS_ESP_lastVoiceEnergy_MASK 0x00000100
#define AWE_AVS_ESP_lastVoiceEnergy_SIZE -1

// fract32 lastAmbientEnergy - VoiceEnergy result from last calculation
#define AWE_AVS_ESP_lastAmbientEnergy_OFFSET 9
#define AWE_AVS_ESP_lastAmbientEnergy_MASK 0x00000200
#define AWE_AVS_ESP_lastAmbientEnergy_SIZE -1

// int lastVADFlag - VoiceEnergy result from last calculation
#define AWE_AVS_ESP_lastVADFlag_OFFSET 10
#define AWE_AVS_ESP_lastVADFlag_MASK 0x00000400
#define AWE_AVS_ESP_lastVADFlag_SIZE -1

// fract16 * pProcessingBuffer - Points to a buffer for algorithm
//         processing
#define AWE_AVS_ESP_pProcessingBuffer_OFFSET 11
#define AWE_AVS_ESP_pProcessingBuffer_MASK 0x00000800
#define AWE_AVS_ESP_pProcessingBuffer_SIZE -1

// STATIC_VARS * pGlobals - Points to the global variables structure
#define AWE_AVS_ESP_pGlobals_OFFSET 12
#define AWE_AVS_ESP_pGlobals_MASK 0x00001000
#define AWE_AVS_ESP_pGlobals_SIZE -1


// ----------------------------------------------------------------------
// VR_State [SourceInt]
// Source buffer holding 1 wire of integer data

#define AWE_VR_State_ID 30006

// int value[1] - Array of interleaved audio data
// Default value:
//     0
// Range: unrestricted
#define AWE_VR_State_value_OFFSET 8
#define AWE_VR_State_value_MASK 0x00000100
#define AWE_VR_State_value_SIZE 1


// ----------------------------------------------------------------------
// AEC_Out_Delay [Delay]
// Time delay in which the delay is specified in samples

#define AWE_AEC_Out_Delay_ID 30004

// int maxDelay - Maximum delay, in samples. The size of the delay
//         buffer is (maxDelay+1)*numChannels.
#define AWE_AEC_Out_Delay_maxDelay_OFFSET 8
#define AWE_AEC_Out_Delay_maxDelay_MASK 0x00000100
#define AWE_AEC_Out_Delay_maxDelay_SIZE -1

// int currentDelay - Current delay.
// Default value: 0
// Range: 0 to 130.  Step size = 1
#define AWE_AEC_Out_Delay_currentDelay_OFFSET 9
#define AWE_AEC_Out_Delay_currentDelay_MASK 0x00000200
#define AWE_AEC_Out_Delay_currentDelay_SIZE -1

// int stateIndex - Index of the oldest state variable in the array of
//         state variables.
#define AWE_AEC_Out_Delay_stateIndex_OFFSET 10
#define AWE_AEC_Out_Delay_stateIndex_MASK 0x00000400
#define AWE_AEC_Out_Delay_stateIndex_SIZE -1

// int stateHeap - Heap in which to allocate memory.
#define AWE_AEC_Out_Delay_stateHeap_OFFSET 11
#define AWE_AEC_Out_Delay_stateHeap_MASK 0x00000800
#define AWE_AEC_Out_Delay_stateHeap_SIZE -1

// float state[133] - State variable array.
#define AWE_AEC_Out_Delay_state_OFFSET 12
#define AWE_AEC_Out_Delay_state_MASK 0x00001000
#define AWE_AEC_Out_Delay_state_SIZE 133


// ----------------------------------------------------------------------
// outputToWaves.recordVR [BooleanSource]
// Source buffer holding 1 wire of data

#define AWE_outputToWaves____recordVR_ID 31004

// int value[1] - Array of interleaved audio real data.
// Default value:
//     0
// Range: 0 to 1.  Step size = 1
#define AWE_outputToWaves____recordVR_value_OFFSET 8
#define AWE_outputToWaves____recordVR_value_MASK 0x00000100
#define AWE_outputToWaves____recordVR_value_SIZE 1


// ----------------------------------------------------------------------
// outputToWaves.recordAEC [BooleanSource]
// Source buffer holding 1 wire of data

#define AWE_outputToWaves____recordAEC_ID 31003

// int value[1] - Array of interleaved audio real data.
// Default value:
//     0
// Range: 0 to 1.  Step size = 1
#define AWE_outputToWaves____recordAEC_value_OFFSET 8
#define AWE_outputToWaves____recordAEC_value_MASK 0x00000100
#define AWE_outputToWaves____recordAEC_value_SIZE 1


// ----------------------------------------------------------------------
// outputToWaves.recordBF [BooleanSource]
// Source buffer holding 1 wire of data

#define AWE_outputToWaves____recordBF_ID 31002

// int value[1] - Array of interleaved audio real data.
// Default value:
//     0
// Range: 0 to 1.  Step size = 1
#define AWE_outputToWaves____recordBF_value_OFFSET 8
#define AWE_outputToWaves____recordBF_value_MASK 0x00000100
#define AWE_outputToWaves____recordBF_value_SIZE 1


// ----------------------------------------------------------------------
// outputToWaves.recordUnproc [BooleanSource]
// Source buffer holding 1 wire of data

#define AWE_outputToWaves____recordUnproc_ID 31001

// int value[1] - Array of interleaved audio real data.
// Default value:
//     0
// Range: 0 to 1.  Step size = 1
#define AWE_outputToWaves____recordUnproc_value_OFFSET 8
#define AWE_outputToWaves____recordUnproc_value_MASK 0x00000100
#define AWE_outputToWaves____recordUnproc_value_SIZE 1


// ----------------------------------------------------------------------
// OutputMeter [Meter]
// Peak and RMS meter module

#define AWE_OutputMeter_ID 30005

// int meterType - Operating mode of the meter. Selects between peak and
//         RMS calculations. See the discussion section for more details.
// Default value: 18
// Range: unrestricted
#define AWE_OutputMeter_meterType_OFFSET 8
#define AWE_OutputMeter_meterType_MASK 0x00000100
#define AWE_OutputMeter_meterType_SIZE -1

// float attackTime - Attack time of the meter. Specifies how quickly
//         the meter value rises.
#define AWE_OutputMeter_attackTime_OFFSET 9
#define AWE_OutputMeter_attackTime_MASK 0x00000200
#define AWE_OutputMeter_attackTime_SIZE -1

// float releaseTime - Release time of the meter. Specifies how quickly
//         the meter decays.
#define AWE_OutputMeter_releaseTime_OFFSET 10
#define AWE_OutputMeter_releaseTime_MASK 0x00000400
#define AWE_OutputMeter_releaseTime_SIZE -1

// float attackCoeff - Internal coefficient that realizes the attack
//         time.
#define AWE_OutputMeter_attackCoeff_OFFSET 11
#define AWE_OutputMeter_attackCoeff_MASK 0x00000800
#define AWE_OutputMeter_attackCoeff_SIZE -1

// float releaseCoeff - Internal coefficient that realizes the release
//         time.
#define AWE_OutputMeter_releaseCoeff_OFFSET 12
#define AWE_OutputMeter_releaseCoeff_MASK 0x00001000
#define AWE_OutputMeter_releaseCoeff_SIZE -1

// float value[3] - Array of meter output values, one per channel.
#define AWE_OutputMeter_value_OFFSET 13
#define AWE_OutputMeter_value_MASK 0x00002000
#define AWE_OutputMeter_value_SIZE 3


// ----------------------------------------------------------------------
// awb_vinfo [SourceInt]
// Source buffer holding 1 wire of integer data

#define AWE_awb_vinfo_ID 32767

// int value[4] - Array of interleaved audio data
// Default value:
//      1
//      6
//     13
//      1
// Range: unrestricted
#define AWE_awb_vinfo_value_OFFSET 8
#define AWE_awb_vinfo_value_MASK 0x00000100
#define AWE_awb_vinfo_value_SIZE 4



#endif // AMLOGIC_VUI_SOLUTION_V13A_VOICEONLY_GEN2_TUNINGGUI_ESPSIMPLE_H

