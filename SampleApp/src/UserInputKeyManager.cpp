/*
 * UserInputKeyManager.cpp
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

#include "SampleApp/UserInputKeyManager.h"
#include "SampleApp/ConsolePrinter.h"
#include "SampleApp/KeyEvent.h"
#include "SampleApp/KeyEventProcess.h"

#include <cctype>

namespace alexaClientSDK {
namespace sampleApp {

std::unique_ptr<UserInputKeyManager> UserInputKeyManager::create(std::shared_ptr<InteractionManager> interactionManager) {
    if (!interactionManager) {
        ConsolePrinter::simplePrint("Invalid InteractionManager passed to UserInputKeyManager");
        return nullptr;
    }
    return std::unique_ptr<UserInputKeyManager>(new UserInputKeyManager(interactionManager));
}

UserInputKeyManager::UserInputKeyManager(std::shared_ptr<InteractionManager> interactionManager) :
        m_interactionKeyManager{interactionManager} {
}

void UserInputKeyManager::run() {
    const char* keyValue = NULL;
    if (!m_interactionKeyManager) {
        return;
    }
    //m_interactionKeyManager->begin();
    EventsProcess* ep = new EventsProcess();
    ep->Init();
    while (true) {
        keyValue = ep->WaitKey();
	if ( !strcmp(keyValue,"mute")) {
            m_interactionKeyManager->microphoneToggle();
	}
    }
}

} // namespace sampleApp
} // namespace alexaClientSDK
