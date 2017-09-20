/*
 * UserInputManager.cpp
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

#include "SampleApp/UserInputManager.h"
#include "SampleApp/ConsolePrinter.h"

#include <cctype>

namespace alexaClientSDK {
namespace sampleApp {

static const char HOLD = 'h';
static const char TAP = 't';
static const char QUIT = 'q';
static const char INFO = 'i';
static const char MIC_TOGGLE = 'm';
static const char STOP = 's';

std::unique_ptr<UserInputManager> UserInputManager::create(std::shared_ptr<InteractionManager> interactionManager) {
    if (!interactionManager) {
        ConsolePrinter::simplePrint("Invalid InteractionManager passed to UserInputManager");
        return nullptr;
    }
    return std::unique_ptr<UserInputManager>(new UserInputManager(interactionManager));
}

UserInputManager::UserInputManager(std::shared_ptr<InteractionManager> interactionManager) : 
        m_interactionManager{interactionManager} { 
}

void UserInputManager::setMode(const string& rmode) {
    std::cout<<"UserInputManager runMode:"<<rmode<<std::endl;
    menuMode = rmode;
}

void UserInputManager::run() {
    char x;
    if (!m_interactionManager) {
        return;
    }
    m_interactionManager->begin(menuMode);
    while(true) {
        if (menuMode == "front") {
            std::cin >> x;
            x = ::tolower(x);
            if (x == QUIT) {
                m_interactionManager->sampleapp_exit();
                return;
            } else if (x == INFO) {
                m_interactionManager->help();
            } else if (x == MIC_TOGGLE) {
                m_interactionManager->microphoneToggle();
            } else if (x == HOLD) {
                m_interactionManager->holdToggled();
            } else if (x == TAP) {
                m_interactionManager->tap();
            } else if (x == STOP) {
                m_interactionManager->stopForegroundActivity();
            }
        } else {
            sleep(10);
        }
    }
}

} // namespace sampleApp
} // namespace alexaClientSDK
