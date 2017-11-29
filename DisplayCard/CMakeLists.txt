cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
file(GLOB DISPLAYCARDLIB "${AlexaClientSDK_SOURCE_DIR}/DisplayCard/*.so")
file(GLOB DISPLAYCARD "${AlexaClientSDK_SOURCE_DIR}/DisplayCard/DisplayCardsD")
install(FILES       ${DISPLAYCARDLIB}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/../usr/lib/")
install(PROGRAMS      ${DISPLAYCARD}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/../sbin/")
