/*
 * DefaultClient.cpp
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

#include "DefaultClient/DefaultClient.h"

#include <ADSL/MessageInterpreter.h>
#include <ACL/Transport/HTTP2MessageRouter.h>
#include <ACL/Transport/PostConnectObject.h>
#include <AVSCommon/AVS/Attachment/AttachmentManager.h>
#include <AVSCommon/AVS/ExceptionEncounteredSender.h>
#include <Settings/SettingsUpdatedEventSender.h>
#include <ContextManager/ContextManager.h>
#include <System/EndpointHandler.h>
#include <System/UserInactivityMonitor.h>

namespace alexaClientSDK {
namespace defaultClient {

/// String to identify log entries originating from this file.
static const std::string TAG("DefaultClient");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

std::unique_ptr<DefaultClient> DefaultClient::create(
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> speakMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> audioMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> alertsMediaPlayer,
    std::shared_ptr<avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<capabilityAgents::alerts::storage::AlertStorageInterface> alertStorage,
    std::shared_ptr<capabilityAgents::settings::SettingsStorageInterface> settingsStorage,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::DialogUXStateObserverInterface>>
        alexaDialogStateObservers,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface>>
        connectionObservers) {
    std::unique_ptr<DefaultClient> defaultClient(new DefaultClient());
    if (!defaultClient->initialize(
            speakMediaPlayer,
            audioMediaPlayer,
            alertsMediaPlayer,
            authDelegate,
            alertStorage,
            settingsStorage,
            alexaDialogStateObservers,
            connectionObservers)) {
        return nullptr;
    }

    return defaultClient;
}

bool DefaultClient::initialize(
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> speakMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> audioMediaPlayer,
    std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface> alertsMediaPlayer,
    std::shared_ptr<avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<capabilityAgents::alerts::storage::AlertStorageInterface> alertStorage,
    std::shared_ptr<capabilityAgents::settings::SettingsStorageInterface> settingsStorage,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::DialogUXStateObserverInterface>>
        alexaDialogStateObservers,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface>>
        connectionObservers) {
    if (!speakMediaPlayer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullSpeakMediaPlayer"));
        return false;
    }

    if (!audioMediaPlayer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAudioMediaPlayer"));
        return false;
    }

    if (!alertsMediaPlayer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAlertsMediaPlayer"));
        return false;
    }

    if (!authDelegate) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "nullAuthDelegate"));
        return false;
    }

    m_dialogUXStateAggregator = std::make_shared<avsCommon::avs::DialogUXStateAggregator>();

    for (auto observer : alexaDialogStateObservers) {
        m_dialogUXStateAggregator->addObserver(observer);
    }

    /*
     * Creating the Focus Manager - This component deals with the management of layered audio focus across various
     * components. It handles granting access to Channels as well as pushing different "Channels" to foreground,
     * background, or no focus based on which other Channels are active and the priorities of those Channels. Each
     * Capability Agent will require the Focus Manager in order to request access to the Channel it wishes to play on.
     */
    m_focusManager = std::make_shared<afml::FocusManager>();

    /*
     * Creating the Attachment Manager - This component deals with managing attachments and allows for readers and
     * writers to be created to handle the attachment.
     */
    auto attachmentManager = std::make_shared<avsCommon::avs::attachment::AttachmentManager>(
        avsCommon::avs::attachment::AttachmentManager::AttachmentType::IN_PROCESS);

    /*
     * Creating the message router - This component actually maintains the connection to AVS over HTTP2. It is created
     * using the auth delegate, which provides authorization to connect to AVS, and the attachment manager, which helps
     * ACL write attachments received from AVS.
     */
    m_messageRouter = std::make_shared<acl::HTTP2MessageRouter>(authDelegate, attachmentManager);

    /*
     * Creating the connection manager - This component is the overarching connection manager that glues together all
     * the other networking components into one easy-to-use component.
     */
    m_connectionManager =
        acl::AVSConnectionManager::create(m_messageRouter, false, connectionObservers, {m_dialogUXStateAggregator});
    if (!m_connectionManager) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateConnectionManager"));
        return false;
    }

    /*
     * Creating our certified sender - this component guarantees that messages given to it (expected to be JSON
     * formatted AVS Events) will be sent to AVS.  This nicely decouples strict message sending from components which
     * require an Event be sent, even in conditions when there is no active AVS connection.
     */
    auto messageStorage = std::make_shared<certifiedSender::SQLiteMessageStorage>();
    m_certifiedSender =
        certifiedSender::CertifiedSender::create(m_connectionManager, m_connectionManager, messageStorage);
    if (!m_certifiedSender) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateCertifiedSender"));
        return false;
    }

    /*
     * Creating the Exception Sender - This component helps the SDK send exceptions when it is unable to handle a
     * directive sent by AVS. For that reason, the Directive Sequencer and each Capability Agent will need this
     * component.
     */
    std::shared_ptr<avsCommon::avs::ExceptionEncounteredSender> exceptionSender =
        avsCommon::avs::ExceptionEncounteredSender::create(m_connectionManager);
    if (!exceptionSender) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateExceptionSender"));
        return false;
    }

    /*
     * Creating the Directive Sequencer - This is the component that deals with the sequencing and ordering of
     * directives sent from AVS and forwarding them along to the appropriate Capability Agent that deals with
     * directives in that Namespace/Name.
     */
    m_directiveSequencer = adsl::DirectiveSequencer::create(exceptionSender);
    if (!m_directiveSequencer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateDirectiveSequencer"));
        return false;
    }

    /*
     * Creating the Message Interpreter - This component takes care of converting ACL messages to Directives for the
     * Directive Sequencer to process. This essentially "glues" together the ACL and ADSL.
     */
    auto messageInterpreter =
        std::make_shared<adsl::MessageInterpreter>(exceptionSender, m_directiveSequencer, attachmentManager);

    m_connectionManager->addMessageObserver(messageInterpreter);

    /*
     * Creating the Context Manager - This component manages the context of each of the components to update to AVS.
     * It is required for each of the capability agents so that they may provide their state just before any event is
     * fired off.
     */
    auto contextManager = contextManager::ContextManager::create();
    if (!contextManager) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateContextManager"));
        return false;
    }
    acl::PostConnectObject::init(contextManager);

    /*
     * Creating the User Inactivity Monitor - This component is responsibly for updating AVS of user inactivity as
     * described in the System Interface of AVS.
     */
    auto userInactivityMonitor =
        capabilityAgents::system::UserInactivityMonitor::create(m_connectionManager, exceptionSender);
    if (!userInactivityMonitor) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateUserInactivityMonitor"));
        return false;
    }

    /*
     * Creating the Audio Input Processor - This component is the Capability Agent that implments the SpeechRecognizer
     * interface of AVS.
     */
    m_audioInputProcessor = capabilityAgents::aip::AudioInputProcessor::create(
        m_directiveSequencer,
        m_connectionManager,
        contextManager,
        m_focusManager,
        m_dialogUXStateAggregator,
        exceptionSender,
        userInactivityMonitor);
    if (!m_audioInputProcessor) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAudioInputProcessor"));
        return false;
    }

    m_audioInputProcessor->addObserver(m_dialogUXStateAggregator);

    /*
     * Creating the Speech Synthesizer - This component is the Capability Agent that implements the SpeechSynthesizer
     * interface of AVS.
     */
    m_speechSynthesizer = capabilityAgents::speechSynthesizer::SpeechSynthesizer::create(
        speakMediaPlayer, m_connectionManager, m_focusManager, contextManager, attachmentManager, exceptionSender);
    if (!m_speechSynthesizer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateSpeechSynthesizer"));
        return false;
    }

    m_speechSynthesizer->addObserver(m_dialogUXStateAggregator);

    /*
     * Creating the Audio Player - This component is the Capability Agent that implements the AudioPlayer
     * interface of AVS.
     */
    m_audioPlayer = capabilityAgents::audioPlayer::AudioPlayer::create(
        audioMediaPlayer, m_connectionManager, m_focusManager, contextManager, attachmentManager, exceptionSender);
    if (!m_audioPlayer) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAudioPlayer"));
        return false;
    }

    /*
     * Creating the Alerts Capability Agent - This component is the Capability Agent that implements the Alerts
     * interface of AVS.
     */
    m_alertsCapabilityAgent = capabilityAgents::alerts::AlertsCapabilityAgent::create(
        m_connectionManager,
        m_certifiedSender,
        m_focusManager,
        contextManager,
        exceptionSender,
        alertStorage,
        capabilityAgents::alerts::renderer::Renderer::create(alertsMediaPlayer));
    if (!m_alertsCapabilityAgent) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateAlertsCapabilityAgent"));
        return false;
    }

    addConnectionObserver(m_alertsCapabilityAgent);

    addConnectionObserver(m_dialogUXStateAggregator);

    /*
     * Creating the PlaybackController Capability Agent - This component is the Capability Agent that implements the
     * PlaybackController interface of AVS.
     */
    m_playbackController =
        capabilityAgents::playbackController::PlaybackController::create(contextManager, m_connectionManager);
    if (!m_playbackController) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreatePlaybackController"));
        return false;
    }

    std::shared_ptr<capabilityAgents::settings::SettingsUpdatedEventSender> settingsUpdatedEventSender =
        alexaClientSDK::capabilityAgents::settings::SettingsUpdatedEventSender::create(m_connectionManager);
    if (!settingsUpdatedEventSender) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateSettingsObserver"));
        return false;
    }

    /*
     * Creating the Setting object - This component implements the Setting interface of AVS.
     */
    m_settings = capabilityAgents::settings::Settings::create(settingsStorage, {settingsUpdatedEventSender});

    if (!m_settings) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateSettingsObject"));
        return false;
    }

    /*
     * Creating the Endpoint Handler - This component is responsible for handling directives from AVS instructing the
     * client to change the endpoint to connect to.
     */
    auto endpointHandler = capabilityAgents::system::EndpointHandler::create(m_connectionManager, exceptionSender);
    if (!endpointHandler) {
        ACSDK_ERROR(LX("initializeFailed").d("reason", "unableToCreateEndpointHandler"));
        return false;
    }

    /*
     * The following two statements show how to register capability agents to the directive sequencer.
     */
    if (!m_directiveSequencer->addDirectiveHandler(m_speechSynthesizer)) {
        ACSDK_ERROR(LX("initializeFailed")
                        .d("reason", "unableToRegisterDirectiveHandler")
                        .d("directiveHandler", "SpeechSynthesizer"));
        return false;
    }

    if (!m_directiveSequencer->addDirectiveHandler(m_audioPlayer)) {
        ACSDK_ERROR(LX("initializeFailed")
                        .d("reason", "unableToRegisterDirectiveHandler")
                        .d("directiveHandler", "AudioPlayer"));
        return false;
    }

    if (!m_directiveSequencer->addDirectiveHandler(m_audioInputProcessor)) {
        ACSDK_ERROR(LX("initializeFailed")
                        .d("reason", "unableToRegisterDirectiveHandler")
                        .d("directiveHandler", "AudioInputProcessor"));
        return false;
    }

    if (!m_directiveSequencer->addDirectiveHandler(m_alertsCapabilityAgent)) {
        ACSDK_ERROR(LX("initializeFailed")
                        .d("reason", "unableToRegisterDirectiveHandler")
                        .d("directiveHandler", "AlertsCapabilityAgent"));
        return false;
    }

    if (!m_directiveSequencer->addDirectiveHandler(endpointHandler)) {
        ACSDK_ERROR(LX("initializeFailed")
                        .d("reason", "unableToRegisterDirectiveHandler")
                        .d("directiveHandler", "EndpointHandler"));
        return false;
    }

    if (!m_directiveSequencer->addDirectiveHandler(userInactivityMonitor)) {
        ACSDK_ERROR(LX("initializeFailed")
                        .d("reason", "unableToRegisterDirectiveHandler")
                        .d("directiveHandler", "UserInactivityMonitor"));
        return false;
    }

    return true;
}

void DefaultClient::connect(const std::string& avsEndpoint) {
    m_connectionManager->setAVSEndpoint(avsEndpoint);
    m_connectionManager->enable();
}

void DefaultClient::disconnect() {
    m_connectionManager->disable();
}

void DefaultClient::stopForegroundActivity() {
    m_focusManager->stopForegroundActivity();
}

void DefaultClient::addAlexaDialogStateObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::DialogUXStateObserverInterface> observer) {
    m_dialogUXStateAggregator->addObserver(observer);
}

void DefaultClient::removeAlexaDialogStateObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::DialogUXStateObserverInterface> observer) {
    m_dialogUXStateAggregator->removeObserver(observer);
}

void DefaultClient::addConnectionObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface> observer) {
    m_connectionManager->addConnectionStatusObserver(observer);
}

void DefaultClient::removeConnectionObserver(
    std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface> observer) {
    m_connectionManager->removeConnectionStatusObserver(observer);
}

void DefaultClient::addSettingObserver(
    const std::string& key,
    std::shared_ptr<avsCommon::sdkInterfaces::SingleSettingObserverInterface> observer) {
    m_settings->addSingleSettingObserver(key, observer);
}

void DefaultClient::removeSettingObserver(
    const std::string& key,
    std::shared_ptr<avsCommon::sdkInterfaces::SingleSettingObserverInterface> observer) {
    m_settings->removeSingleSettingObserver(key, observer);
}

void DefaultClient::changeSetting(const std::string& key, const std::string& value) {
    m_settings->changeSetting(key, value);
}

void DefaultClient::sendDefaultSettings() {
    m_settings->sendDefaultSettings();
}

avsCommon::sdkInterfaces::PlaybackControllerInterface& DefaultClient::getPlaybackControllerInterface() const {
    return *m_playbackController;
}

std::future<bool> DefaultClient::notifyOfWakeWord(
    capabilityAgents::aip::AudioProvider wakeWordAudioProvider,
    avsCommon::avs::AudioInputStream::Index beginIndex,
    avsCommon::avs::AudioInputStream::Index endIndex,
    std::string keyword) {
    return m_audioInputProcessor->recognize(
        wakeWordAudioProvider, capabilityAgents::aip::Initiator::WAKEWORD, beginIndex, endIndex, keyword);
}

std::future<bool> DefaultClient::notifyOfTapToTalk(
    capabilityAgents::aip::AudioProvider tapToTalkAudioProvider,
    avsCommon::avs::AudioInputStream::Index beginIndex) {
    return m_audioInputProcessor->recognize(tapToTalkAudioProvider, capabilityAgents::aip::Initiator::TAP, beginIndex);
}

std::future<bool> DefaultClient::notifyOfHoldToTalkStart(capabilityAgents::aip::AudioProvider holdToTalkAudioProvider) {
    return m_audioInputProcessor->recognize(holdToTalkAudioProvider, capabilityAgents::aip::Initiator::PRESS_AND_HOLD);
}

std::future<bool> DefaultClient::notifyOfHoldToTalkEnd() {
    return m_audioInputProcessor->stopCapture();
}

DefaultClient::~DefaultClient() {
    if (m_directiveSequencer) {
        m_directiveSequencer->shutdown();
    }
    if (m_audioInputProcessor) {
        m_audioInputProcessor->shutdown();
    }
    if (m_audioPlayer) {
        m_audioPlayer->shutdown();
    }
    if (m_speechSynthesizer) {
        m_speechSynthesizer->shutdown();
    }
    if (m_alertsCapabilityAgent) {
        m_alertsCapabilityAgent->shutdown();
    }
    if (m_playbackController) {
        m_playbackController->shutdown();
    }
    if (m_messageRouter) {
        m_messageRouter->shutdown();
    }
    if (m_connectionManager) {
        m_connectionManager->shutdown();
    }
    if (m_certifiedSender) {
        m_certifiedSender->shutdown();
    }
}

}  // namespace defaultClient
}  // namespace alexaClientSDK
