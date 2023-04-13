
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

add_executable(simplest_video_play_sdl2 simplest_video_play_sdl2.cpp
	${COMMON_SOURCE_FILES} 
	${COMMON_SDL_SOURCE_FILES}
)
target_include_directories(simplest_video_play_sdl2 PRIVATE 
	${COMMON_FILE_DIR} 
	"${COMMON_FILE_DIR}/SDL2" 
	${SDL2_INCLUDE_DIR}
)
target_link_directories(simplest_video_play_sdl2 PRIVATE ${SDL2_LIBDIR})
target_link_libraries(simplest_video_play_sdl2 PRIVATE sdl2.lib sdl2main.lib)

add_executable(test_player test_player.cpp
	${COMMON_SOURCE_FILES}
	${COMMON_SDL_SOURCE_FILES} 
	${COMMON_CONVERT_FILES}
	${COMMON_FFMPEG_SOURCE_FILES} 
	"${COMMON_FILE_DIR}/SDL2/RenderHelper/RenderHelper.h"
	"${COMMON_FILE_DIR}/SDL2/RenderHelper/SDLRenderHelper.h"
	"${COMMON_FILE_DIR}/SDL2/RenderHelper/SDLRenderHelper.cpp"
)
target_include_directories(test_player PRIVATE
	${FFMPEG_INCLUDE_DIR} 
	${SDL2_INCLUDE_DIR}
	${COMMON_FILE_DIR}
	"${COMMON_FILE_DIR}/SDL2"
	"${COMMON_FILE_DIR}/SDL2/RenderHelper"
	"${COMMON_FILE_DIR}/FFmpeg"
)
target_link_directories(test_player PRIVATE
	${FFMPEG_LIB_DIR} 
	${SDL2_LIBDIR}
)
target_link_libraries(test_player PRIVATE
	avcodec.lib avutil.lib avformat.lib swscale.lib
	sdl2.lib sdl2main.lib
)

enable_language(CUDA)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_ARCHITECTURES "86")
find_package(CUDAToolkit REQUIRED)
add_executable(simplest_ffmpeg_player simplest_ffmpeg_player.cpp 
	${COMMON_SOURCE_FILES}
	${COMMON_SDL_SOURCE_FILES} 
	${COMMON_RENDERHELPER_SOURCE_FILES}
	${COMMON_FFMPEG_SOURCE_FILES} 
	${COMMON_CONVERT_FILES}
	"${COMMON_FILE_DIR}/FFmpeg/ColorSpace.cu"
)
target_include_directories(simplest_ffmpeg_player PRIVATE 
	${FFMPEG_INCLUDE_DIR} 
	${SDL2_INCLUDE_DIR}
	${COMMON_FILE_DIR}
	"${COMMON_FILE_DIR}/SDL2"
	"${COMMON_FILE_DIR}/SDL2/RenderHelper"
	"${COMMON_FILE_DIR}/FFmpeg"
	${GLEW_INCLUDE_DIR}
	${CUDAToolkit_INCLUDE_DIRS}
)
target_link_directories(simplest_ffmpeg_player PRIVATE
	${FFMPEG_LIB_DIR} 
	${SDL2_LIBDIR}
	${GLEW_LIB_DIR}
	${CUDAToolkit_LIBRARY_DIR}
)
target_link_libraries(simplest_ffmpeg_player PRIVATE
	avcodec.lib avutil.lib avformat.lib swscale.lib
	sdl2.lib sdl2main.lib
	glew32.lib
	opengl32.lib
	CUDA::cudart_static
)
#separable compilation for cuda
set_target_properties(simplest_ffmpeg_player PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

add_executable(simplest_ffmpeg_player_su simplest_ffmpeg_player_su.cpp ${COMMON_SOURCE_FILES} ${COMMON_SDL_SOURCE_FILES})
target_include_directories(simplest_ffmpeg_player_su PRIVATE ${FFMPEG_INCLUDE_DIR} ${COMMON_FILE_DIR} "${COMMON_FILE_DIR}/SDL2" ${SDL2_INCLUDE_DIR})
target_link_directories(simplest_ffmpeg_player_su PRIVATE ${FFMPEG_LIB_DIR} ${SDL2_LIBDIR})
target_link_libraries(simplest_ffmpeg_player_su PRIVATE avcodec.lib avutil.lib avformat.lib swscale.lib sdl2.lib sdl2main.lib)

#[[
add_executable(hw_decode hw_decode.cpp)
target_include_directories(hw_decode PRIVATE ${FFMPEG_INCLUDE_DIR})
target_link_directories(hw_decode PRIVATE ${FFMPEG_LIB_DIR})
target_link_libraries(hw_decode PRIVATE avcodec.lib avutil.lib avformat.lib) 
]]

add_executable(simplest_ffmpeg_audio_decoder simplest_ffmpeg_audio_decoder.cpp
	${COMMON_SOURCE_FILES}
	${COMMON_SDL_SOURCE_FILES} 
	${COMMON_FFMPEG_SOURCE_FILES} 
	${COMMON_AUDIO_SOURCE_FILES}
)
target_include_directories(simplest_ffmpeg_audio_decoder PRIVATE
	${FFMPEG_INCLUDE_DIR} 
	${SDL2_INCLUDE_DIR}
	${COMMON_FILE_DIR}
	"${COMMON_FILE_DIR}/SDL2"
	"${COMMON_FILE_DIR}/FFmpeg"
	"${COMMON_FILE_DIR}/Audio"
)
target_link_directories(simplest_ffmpeg_audio_decoder PRIVATE
	${FFMPEG_LIB_DIR} 
	${SDL2_LIBDIR}
)
target_link_libraries(simplest_ffmpeg_audio_decoder PRIVATE
	avcodec.lib avutil.lib avformat.lib swscale.lib swresample.lib
	sdl2.lib sdl2main.lib
)

add_executable(sdl_pcm_player sdl_pcm_player.cpp)
target_include_directories(sdl_pcm_player PRIVATE
	${SDL2_INCLUDE_DIR}
)
target_link_directories(sdl_pcm_player PRIVATE
	${SDL2_LIBDIR}
)
target_link_libraries(sdl_pcm_player PRIVATE
	sdl2.lib sdl2main.lib
)
