add_library(DSP SHARED
    DSPKeyWordDetector.cpp)

include_directories(
	"${DSP_KEY_WORD_DETECTOR_INCLUDE_DIR}"
	"${KWD_SOURCE_DIR}/include"
	"${AVSCommon_SOURCE_DIR}/include"
	)

target_link_libraries(DSP KWD AVSCommon "${DSP_KEY_WORD_DETECTOR_LIB_PATH}")
# install target
asdk_install()
 
# these files are needed by DSP KWD
install(PROGRAMS    "${DSP_KEY_WORD_DETECTOR_LIB_PATH}"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/"
        RENAME      "AWELib.so")
file(GLOB RAWFILES "${AlexaClientSDK_SOURCE_DIR}/KWD/DSP/model/*.raw")
file(GLOB AWBFILES "${AlexaClientSDK_SOURCE_DIR}/KWD/DSP/model/*.awb")
file(GLOB MP3FILES "${AlexaClientSDK_SOURCE_DIR}/Media/*.mp3")
install(PROGRAMS    ${RAWFILES} ${AWBFILES}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
install(FILES       ${MP3FILES}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/avs")
