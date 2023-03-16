
add_executable(simplest_ffmpeg_decoder simplest_ffmpeg_decoder.cpp)
target_include_directories(simplest_ffmpeg_decoder PRIVATE ${FFMPEG_INCLUDE_DIR})
target_link_directories(simplest_ffmpeg_decoder PRIVATE ${FFMPEG_LIB_DIR})
target_link_libraries(simplest_ffmpeg_decoder PRIVATE avcodec.lib avformat.lib avutil.lib swscale.lib)

add_executable(simplest_ffmpeg_decoder_v1 simplest_ffmpeg_decoder_v1.cpp ${COMMON_SOURCE_FILES})
target_include_directories(simplest_ffmpeg_decoder_v1 PRIVATE ${FFMPEG_INCLUDE_DIR} ${COMMON_FILE_DIR})
target_link_directories(simplest_ffmpeg_decoder_v1 PRIVATE ${FFMPEG_LIB_DIR})
target_link_libraries(simplest_ffmpeg_decoder_v1 PRIVATE avcodec.lib avformat.lib avutil.lib swscale.lib)

add_executable(simplest_ffmpeg_decoder_pure simplest_ffmpeg_decoder_pure.cpp ${COMMON_SOURCE_FILES})
target_include_directories(simplest_ffmpeg_decoder_pure PRIVATE ${FFMPEG_INCLUDE_DIR} ${COMMON_FILE_DIR})
target_link_directories(simplest_ffmpeg_decoder_pure PRIVATE ${FFMPEG_LIB_DIR})
target_link_libraries(simplest_ffmpeg_decoder_pure PRIVATE avcodec.lib avutil.lib)

