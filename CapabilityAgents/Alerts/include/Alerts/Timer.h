/*
 * Timer.h
 *
 * Copyright 2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXA_CLIENT_SDK_CAPABILITY_AGENTS_ALERTS_INCLUDE_ALERTS_TIMER_H_
#define ALEXA_CLIENT_SDK_CAPABILITY_AGENTS_ALERTS_INCLUDE_ALERTS_TIMER_H_

#include "Alerts/Alert.h"

namespace alexaClientSDK {
namespace capabilityAgents {
namespace alerts {

/**
 * A Timer class.  This represents an alert which the user wishes to activate at a point in time relative to the
 * current time.  This is different from requesting an alert at an absolute point in time.
 *
 * Timers may be basic, or named.  If named, they will use custom assets at the point of activation.
 *
 * Example of basic timer use:
 * "Alexa, set a timer for 10 seconds."
 * 10 seconds later : device will render a simple audio file, local to the device, to alert the user.
 *
 * Example of named timer use:
 * "Alexa, set an egg timer for 10 seconds"
 * 10 seconds later : Alexa will say something like "Your egg timer is complete".
 */
class Timer : public Alert {
public:
    /// String representation of this type.
    static const std::string TYPE_NAME;

    /**
     * A static function to set the default audio file path for this type of alert.
     *
     * @note This function should only be called at initialization, before any objects have been instantiated.
     *
     * @param filePath The path to the audio file.
     */
    static void setDefaultAudioFilePath(const std::string& filePath);

    /**
     * A static function to set the short audio file path for this type of alert.
     *
     * @note This function should only be called at initialization, before any objects have been instantiated.
     *
     * @param filePath The path to the audio file.
     */
    static void setDefaultShortAudioFilePath(const std::string& filePath);

    std::string getDefaultAudioFilePath() const override;

    std::string getDefaultShortAudioFilePath() const override;

    std::string getTypeName() const override;

private:
    /// The class-level audio file path.
    static std::string m_defaultAudioFilePath;
    /// The class-level short audio file path.
    static std::string m_defaultShortAudioFilePath;
};

}  // namespace alerts
}  // namespace capabilityAgents
}  // namespace alexaClientSDK

#endif  // ALEXA_CLIENT_SDK_CAPABILITY_AGENTS_ALERTS_INCLUDE_ALERTS_TIMER_H_