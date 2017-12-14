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
#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>
#include "SampleApp/UserInputKeyManager.h"
#include "SampleApp/ConsolePrinter.h"
#include "SampleApp/KeyEvent.h"
#include "SampleApp/KeyEventProcess.h"

#include <cctype>

namespace alexaClientSDK {
namespace sampleApp {

using namespace avsCommon::sdkInterfaces;
static const int8_t INCREASE_VOLUME = 10;
static const int8_t DECREASE_VOLUME = -10;

std::unique_ptr<UserInputKeyManager> UserInputKeyManager::create(std::shared_ptr<InteractionManager> interactionManager) {
    if (!interactionManager) {
        ConsolePrinter::simplePrint("Invalid InteractionManager passed to UserInputKeyManager");
        return nullptr;
    }
#ifdef DISPLAYCARD_AML
    std::unique_ptr<UserInputKeyManager> userInputKeyManager(new UserInputKeyManager(interactionManager));

    userInputKeyManager->remote_player_key_init();

    return userInputKeyManager;
#else
    return std::unique_ptr<UserInputKeyManager>(new UserInputKeyManager(interactionManager));
#endif
}

UserInputKeyManager::UserInputKeyManager(std::shared_ptr<InteractionManager> interactionManager) :
        m_interactionKeyManager{interactionManager} {
}

#ifdef DISPLAYCARD_AML
void UserInputKeyManager::remote_player_key_run_thread() {
    int res;
    char* sigvalue;
    while (true) {
        res = disp_state();
        if (res < 0) {
            usleep(1000*200);
            continue;
        }
        res = disp_recv(&sigvalue);
        if (0 == res) {
            //printf("Got Singal with value : %s\n",sigvalue);
            if (!strcmp(sigvalue,"remote_prev_action")) {
                m_interactionKeyManager->playbackPrevious();
            } else if (!strcmp(sigvalue,"remote_play_action")) {
                m_interactionKeyManager->playbackPlay();
            } else if (!strcmp(sigvalue,"remote_next_action")) {
                m_interactionKeyManager->playbackNext();
            } else if (!strcmp(sigvalue,"remote_stop_action")) {
                m_interactionKeyManager->stopForegroundActivity();
            } else if(!strcmp(sigvalue,"remote_tap_action")) {
                m_interactionKeyManager->tap();
            } else if (!strcmp(sigvalue,"remote_pause_action")) {
                m_interactionKeyManager->playbackPause();
            } else ConsolePrinter::simplePrint("remote_null_action!\n");
            sigvalue = NULL;
        }
    }
}

bool UserInputKeyManager::remote_player_key_init() {
    int res = disp_connection(DISPLAYCARD_CLIENT);
    if (-1 == res) return false;

    remote_player_key_thread = std::thread (&UserInputKeyManager::remote_player_key_run_thread,this);

    return true;
}
#endif

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
        if ((keyValue != NULL) && !strcmp(keyValue,"mute")) {
            m_interactionKeyManager->microphoneToggle();
        } else if ((keyValue != NULL) && !strcmp(keyValue,"tap")) {
            m_interactionKeyManager->tap();
        } else if ((keyValue != NULL) && !strcmp(keyValue,"VolumeUp")) {
            m_interactionKeyManager->adjustVolume(SpeakerInterface::Type::AVS_SYNCED, INCREASE_VOLUME);
        } else if ((keyValue != NULL) && !strcmp(keyValue,"VolumeDown")) {
            m_interactionKeyManager->adjustVolume(SpeakerInterface::Type::AVS_SYNCED, DECREASE_VOLUME);
        }
    }
}

} // namespace sampleApp
} // namespace alexaClientSDK
