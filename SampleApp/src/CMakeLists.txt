add_executable(SampleApp 
    main.cpp 
    PortAudioMicrophoneWrapper.cpp 
    InteractionManager.cpp 
    UserInputManager.cpp
    UserInputKeyManager.cpp
    ConsolePrinter.cpp
    GuiRenderer.cpp
    UIManager.cpp
    KeywordObserver.cpp
    ConnectionObserver.cpp
    SampleApplication.cpp
    KeyEvent.cpp
    KeyEventProcess.cpp)

target_include_directories(SampleApp PUBLIC 
    "${SampleApp_SOURCE_DIR}/include"
    "${MediaPlayer_SOURCE_DIR}/include"
    "${DefaultClient_SOURCE_DIR}/include"
    "${AudioResources_SOURCE_DIR}/include"
    "${PORTAUDIO_INCLUDE_DIR}"
    "${LEDRING_INCLUDE_DIR}"
    "${DISPLAYCARD_INCLUDE_DIR}"
    "${KWD_SOURCE_DIR}/include"
    "${KWD_SOURCE_DIR}/DSP/include" )

target_link_libraries(SampleApp 
    DefaultClient
    AuthDelegate 
    MediaPlayer
    "${LEDRING_LIB_PATH}"
    "${DISPLAYCARD_LIB_PATH}"
    "${PORTAUDIO_LIB_PATH}")

if(KITTAI_KEY_WORD_DETECTOR)
    target_link_libraries(SampleApp 
        KITTAI)
endif()

if(SENSORY_KEY_WORD_DETECTOR)
    target_link_libraries(SampleApp SENSORY)
endif()

if(DSP_KEY_WORD_DETECTOR)
    target_link_libraries(SampleApp DSP)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(SampleApp
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework AudioUnit"
        "-framework CoreServices"
        "-framework Carbon")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(SampleApp
        rt m pthread asound)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(FATAL_ERROR "Windows is unsupported at the moment")
endif()
#install target
install(TARGETS     SampleApp
        DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
