
enable_language(CUDA)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_ARCHITECTURES "86")
find_package(CUDAToolkit REQUIRED)

add_executable(playerv1 playerv1.cpp
	${COMMON_SDL_SOURCE_FILES}
	${COMMON_RENDERHELPER_SOURCE_FILES}
	${COMMON_FFMPEG_SOURCE_FILES}
	${COMMON_AUDIO_SOURCE_FILES}
	${COMMON_CONVERT_FILES}
	"${COMMON_FILE_DIR}/FFmpeg/ColorSpace.cu"
)

target_include_directories(playerv1 PRIVATE
	${FFMPEG_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
	${SDL2_INCLUDE_DIR}
	${COMMON_FILE_DIR}
	"${COMMON_FILE_DIR}/SDL2"
	"${COMMON_FILE_DIR}/FFmpeg"
	"${COMMON_FILE_DIR}/SDL2/RenderHelper"
	"${COMMON_FILE_DIR}/Audio"
	${CUDAToolkit_INCLUDE_DIRS}
)

target_link_directories(playerv1 PRIVATE
	${FFMPEG_LIB_DIR}
	${GLEW_LIB_DIR}
	${SDL2_LIBDIR}
	${CUDAToolkit_LIBRARY_DIR}
)

target_link_libraries(playerv1 PRIVATE
	glew32.lib
	opengl32.lib
	sdl2.lib sdl2main.lib
	avutil.lib avcodec.lib avformat.lib swresample.lib
	CUDA::cudart_static
)

set_target_properties(playerv1 PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
