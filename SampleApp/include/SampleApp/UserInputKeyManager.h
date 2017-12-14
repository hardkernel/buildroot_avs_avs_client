/*
 * UserInputKeyManager.h
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

#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_USER_INPUT_KEY_MANAGER_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_USER_INPUT_KEY_MANAGER_H_

#include <memory>
#include "InteractionManager.h"

#ifdef DISPLAYCARD_AML
#include <DisplayCardImpCom.h>
#endif

namespace alexaClientSDK {
namespace sampleApp {

/// Observes user input from the console and notifies the interaction manager of the user's intentions.
class UserInputKeyManager {
public:
    /**
     * Create a UserInputManager.
     *
     * @param interactionManager An instance of the @c InteractionManager used to manage user input.
     * @return Returns a new @c UserInputManager, or @c nullptr if the operation failed.
     */
    static std::unique_ptr<UserInputKeyManager> create(std::shared_ptr<InteractionManager> interactionManager);

    /**
     * Processes user input forever. Returns upon a quit command.
     */
    void run();

#ifdef DISPLAYCARD_AML
    bool remote_player_key_init();
    std::thread remote_player_key_thread;
    void remote_player_key_run_thread();
#endif

private:
    /**
     * Constructor.
     */
    UserInputKeyManager(std::shared_ptr<InteractionManager> interactionManager);

    /// The main interaction manager that interfaces with the SDK.
    std::shared_ptr<InteractionManager> m_interactionManager;

    std::shared_ptr<InteractionManager> m_interactionKeyManager;

};

} // namespace sampleApp
} // namespace alexaClientSDK

#endif // ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_USER_INPUT_MANAGER_H_
