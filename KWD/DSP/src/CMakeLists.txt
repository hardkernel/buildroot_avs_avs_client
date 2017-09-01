add_library(DSP SHARED
    DSPKeyWordDetector.cpp)

include_directories( 
	"../include"
	"{DSP_KEY_WORD_DETECTOR_INCLUDE_DIR}"
	"${KWD_SOURCE_DIR}/include"
	"${AVSCommon_SOURCE_DIR}/include"
	)

target_link_libraries(DSP KWD AVSCommon "${DSP_KEY_WORD_DETECTOR_LIB_PATH}")	

