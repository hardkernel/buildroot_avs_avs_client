/*
 * UIManager.h
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

#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_UI_MANAGER_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_UI_MANAGER_H_

#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/Utils/Threading/Executor.h>

#include "ledClient.h"

namespace alexaClientSDK {
namespace sampleApp {

/**
 * This class manages the states that the user will see when interacting with the Sample Application. For now, it simply
 * prints states to the screen.
 */
class UIManager : 
        public avsCommon::sdkInterfaces::DialogUXStateObserverInterface,
        public avsCommon::sdkInterfaces::ConnectionStatusObserverInterface {
public:
    void onDialogUXStateChanged(DialogUXState state) override;

    void onConnectionStatusChanged(const Status status, const ChangedReason reason) override;

    /**
    * led ring.
    */
    void led_init();
    void led_release();

    /**
     * Prints the welcome screen.
     */
    void printWelcomeScreen();

    /**
     * Prints the help screen.
     */
    void printHelpScreen();

    /**
     * Notifies the user that the microphone is off.
     */
    void microphoneOff();

    /*
     * Prints the state that Alexa is currenty in.
     */
    void microphoneOn();

private:
    /**
     * Prints the current state of Alexa after checking what the appropriate message to display is based on the current
     * component states. This should only be used within the internal executor.
     */
    void printState();
    
    /// The current dialog UX state of the SDK
    DialogUXState m_dialogState;

    /// The current connection state of the SDK.
    avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status m_connectionStatus;

    /// An internal executor that performs execution of callable objects passed to it sequentially but asynchronously.
    avsCommon::utils::threading::Executor m_executor;
};

} // namespace sampleApp
} // namespace alexaClientSDK

#endif // ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_UI_MANAGER_H_
