
add_executable(simplest_ffmpeg_decoder simplest_ffmpeg_decoder.cpp)
target_include_directories(simplest_ffmpeg_decoder PRIVATE ${FFMPEG_INCLUDE_DIR})
target_link_directories(simplest_ffmpeg_decoder PRIVATE ${FFMPEG_LIB_DIR})
target_link_libraries(simplest_ffmpeg_decoder PRIVATE avcodec.lib avformat.lib avutil.lib swscale.lib)

add_custom_command(TARGET simplest_ffmpeg_decoder POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${FFMPEG_BIN_DIR}"
	"${CMAKE_CURRENT_BINARY_DIR}"
)
