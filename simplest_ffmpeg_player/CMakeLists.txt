
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

add_executable(simplest_video_play_sdl2 simplest_video_play_sdl2.cpp ${COMMON_SOURCE_FILES} ${COMMON_SDL_SOURCE_FILES})
target_include_directories(simplest_video_play_sdl2 PRIVATE ${COMMON_FILE_DIR} "${COMMON_FILE_DIR}/SDL2" ${SDL2_INCLUDE_DIR})
target_link_directories(simplest_video_play_sdl2 PRIVATE ${SDL2_LIBDIR})
target_link_libraries(simplest_video_play_sdl2 PRIVATE sdl2.lib sdl2main.lib)

add_executable(simplest_ffmpeg_player simplest_ffmpeg_player.cpp ${COMMON_SOURCE_FILES} ${COMMON_SDL_SOURCE_FILES} ${COMMON_FFMPEG_SOURCE_FILES})
target_include_directories(simplest_ffmpeg_player PRIVATE ${FFMPEG_INCLUDE_DIR} ${SDL2_INCLUDE_DIR} ${COMMON_FILE_DIR} "${COMMON_FILE_DIR}/SDL2" "${COMMON_FILE_DIR}/FFmpeg")
target_link_directories(simplest_ffmpeg_player PRIVATE ${FFMPEG_LIB_DIR} ${SDL2_LIBDIR})
target_link_libraries(simplest_ffmpeg_player PRIVATE avcodec.lib avutil.lib avformat.lib swscale.lib sdl2.lib sdl2main.lib)

add_executable(simplest_ffmpeg_player_su simplest_ffmpeg_player_su.cpp ${COMMON_SOURCE_FILES} ${COMMON_SDL_SOURCE_FILES})
target_include_directories(simplest_ffmpeg_player_su PRIVATE ${FFMPEG_INCLUDE_DIR} ${COMMON_FILE_DIR} "${COMMON_FILE_DIR}/SDL2" ${SDL2_INCLUDE_DIR})
target_link_directories(simplest_ffmpeg_player_su PRIVATE ${FFMPEG_LIB_DIR} ${SDL2_LIBDIR})
target_link_libraries(simplest_ffmpeg_player_su PRIVATE avcodec.lib avutil.lib avformat.lib swscale.lib sdl2.lib sdl2main.lib)
