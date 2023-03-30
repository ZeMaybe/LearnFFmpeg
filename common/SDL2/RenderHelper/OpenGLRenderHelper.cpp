
extern "C"
{
#include "libavcodec/avcodec.h"
}

#include "convert.h"
#include "OpenGLRenderHelper.h"
#include "SDLApp.h"
#include "ColorSpace.h"
#include <chrono>
//#include <nvjpeg.h>
//#include <iostream>
//#include <vector>
//#include <fstream>
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

void glClearError()
{
#ifndef NDEBUG
    while (glGetError() != GL_NO_ERROR);
#endif 
}

void glCheckError()
{
#ifndef NDEBUG
    GLenum error;
    while (error = glGetError())
    {
        SDL_Log("OpenGL error : 0x%X\n", error);
    }
#endif // DEBUG
}

void glLogCall(const char* func, const char* file, int line)
{
#ifndef DEBUG
    char driver[MAX_PATH] = { 0 };
    char dir[MAX_PATH] = { 0 };
    char name[MAX_PATH] = { 0 };
    char ext[MAX_PATH] = { 0 };
    _splitpath(file, driver, dir, name, ext);
    GLenum error;
    while (error = glGetError())
    {
        SDL_Log("OpenGL error : 0x%X : %s:%d.\n%s.\n", error, name, line, func);
    }
#endif 
}

OpenGLRenderHelper::OpenGLRenderHelper(SDL_Window* w)
    :RenderHelper(w)
{
    createRenderer();
}

OpenGLRenderHelper:: ~OpenGLRenderHelper()
{
    clear();
}

#if false
#define CHECK_NVJPEG(S) do {nvjpegStatus_t  status; \
        status = S; \
        if (status != NVJPEG_STATUS_SUCCESS ) std::cout << __LINE__ <<" CHECK_NVJPEG - status = " << status << std::endl; \
        } while (false)

int saveJpeg(const char* filepath, unsigned char* d_srcBGR, int width, int height)
{
    nvjpegHandle_t nvjpeg_handle;
    nvjpegEncoderState_t encoder_state;
    nvjpegEncoderParams_t encoder_params;

    cudaEvent_t ev_start, ev_end;
    cudaEventCreate(&ev_start);
    cudaEventCreate(&ev_end);

    nvjpegImage_t input;
    //nvjpegInputFormat_t input_format = NVJPEG_INPUT_BGRI;
    nvjpegInputFormat_t input_format = NVJPEG_INPUT_RGB;
    int image_width = width;
    int image_height = height;

    input.channel[0] = d_srcBGR;
    //input.pitch[0] = image_width * 3;
    input.pitch[0] = image_width * 3;

    nvjpegBackend_t backend = NVJPEG_BACKEND_DEFAULT;

    CHECK_NVJPEG(nvjpegCreate(backend, nullptr, &nvjpeg_handle));

    CHECK_NVJPEG(nvjpegEncoderParamsCreate(nvjpeg_handle, &encoder_params, NULL));
    CHECK_NVJPEG(nvjpegEncoderStateCreate(nvjpeg_handle, &encoder_state, NULL));

    // set params
    CHECK_NVJPEG(nvjpegEncoderParamsSetEncoding(encoder_params, nvjpegJpegEncoding_t::NVJPEG_ENCODING_PROGRESSIVE_DCT_HUFFMAN, NULL));
    CHECK_NVJPEG(nvjpegEncoderParamsSetOptimizedHuffman(encoder_params, 1, NULL));
    CHECK_NVJPEG(nvjpegEncoderParamsSetQuality(encoder_params, 70, NULL));
    CHECK_NVJPEG(nvjpegEncoderParamsSetSamplingFactors(encoder_params, nvjpegChromaSubsampling_t::NVJPEG_CSS_420, NULL));

    cudaEventRecord(ev_start);
    CHECK_NVJPEG(nvjpegEncodeImage(nvjpeg_handle, encoder_state, encoder_params, &input, input_format, image_width, image_height, NULL));
    cudaEventRecord(ev_end);

    std::vector<unsigned char> obuffer;
    size_t length = 0;
    CHECK_NVJPEG(nvjpegEncodeRetrieveBitstream(
        nvjpeg_handle,
        encoder_state,
        NULL,
        &length,
        NULL));

    obuffer.resize(length);
    CHECK_NVJPEG(nvjpegEncodeRetrieveBitstream(
        nvjpeg_handle,
        encoder_state,
        obuffer.data(),
        &length,
        NULL));

    cudaEventSynchronize(ev_end);

    // ”√ÕÍœ˙ªŸ£¨±‹√‚œ‘¥Ê–π¬∂
    nvjpegEncoderParamsDestroy(encoder_params);
    nvjpegEncoderStateDestroy(encoder_state);
    nvjpegDestroy(nvjpeg_handle);

    float ms;
    cudaEventElapsedTime(&ms, ev_start, ev_end);
    std::cout << "time spend " << ms << " ms" << std::endl;

    std::ofstream outputFile(filepath, std::ios::out | std::ios::binary);
    outputFile.write(reinterpret_cast<const char*>(obuffer.data()), static_cast<int>(length));
    outputFile.close();

    return 0;
}
#endif

void OpenGLRenderHelper::update(AVFrame* frame)
{
    if (frame == nullptr || frame->format != AV_PIX_FMT_CUDA)
        return;

    if (bufferObject == 0)
    {
        createTexture(frame->width, frame->height);
    }
    SDL_GL_MakeCurrent(wnd, glCtx);

    cudaGraphicsResource_t cudaResource;
    auto  re = cudaGraphicsGLRegisterBuffer(&cudaResource, bufferObject, cudaGraphicsRegisterFlagsWriteDiscard);
    if (re != cudaSuccess)
    {
        SDL_Log("can't regisger opengl buffer to cuda.\n");
        SDL_Log("cuda error = %s,error string = %s", cudaGetErrorName(re), cudaGetErrorString(re));
        return;
    }

    re = cudaGraphicsMapResources(1, &cudaResource, 0);
    if (re != cudaSuccess)
    {
        SDL_Log("can't map opengl buffer to cuda.\n");
        SDL_Log("cuda error = %s,error string = %s", cudaGetErrorName(re), cudaGetErrorString(re));
        return;
    }
    void* cudaPointer;
    size_t size = 0;
    re = cudaGraphicsResourceGetMappedPointer(&cudaPointer, &size, cudaResource);
    if (re != cudaSuccess)
    {
        SDL_Log("can't get mapped pointer");
        SDL_Log("cuda error = %s,error string = %s", cudaGetErrorName(re), cudaGetErrorString(re));
        return;
    }

    //auto t1 = std::chrono::high_resolution_clock::now();
    Nv12ToColor32_v2<RGBA32>(frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], (uint8_t*)cudaPointer, frame->width * 4, frame->width, frame->height, frame->colorspace);
    //auto t2 = std::chrono::high_resolution_clock::now();
    //auto d1 = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
    //SDL_Log("time for color space change:%lld", d1.count());

#if false
    static int count = 0;
    char bmpName[32] = { 0 };
    sprintf_s(bmpName, "bmp/%d.jpeg", count++);
    saveJpeg(bmpName, (unsigned char*)cudaPointer, frame->width, frame->height);
    //char* tmpData = new char[frame->width * frame->height * 4];
    //cudaMemcpy(tmpData, cudaPointer, frame->width * frame->height * 4, cudaMemcpyDeviceToHost);
    //stbi_write_bmp(bmpName, frame->width, frame->height, 4, tmpData);
    //delete[]tmpData;
#endif

    cudaGraphicsUnmapResources(1, &cudaResource, 0);
    cudaGraphicsUnregisterResource(cudaResource);
    //AVHWFramesContext* tmpCtx = (AVHWFramesContext*)frame->hw_frames_ctx->data;


    glCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufferObject));
    //const void* t = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_ONLY);
    //glCheckError();
    //stbi_write_bmp(bmpName, frame->width, frame->height, 4, t);
    //glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glCall(glBindTexture(GL_TEXTURE_2D, textureObject));
    glCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, frame->width, frame->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
    //glCall(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width, frame->height, GL_RGBA, GL_UNSIGNED_BYTE, 0));
    glCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
    glCall(glBindTexture(GL_TEXTURE_RECTANGLE, 0));
}

void OpenGLRenderHelper::onRender()
{
    SDL_GL_MakeCurrent(wnd, glCtx);

    int w, h;
    SDL_GetWindowSize(wnd, &w, &h);
    glViewport(0, 0, w, h);

    glCall(glClearColor(1.0f, 0.8f, 0.5f, 0.0f));
    glCall(glClear(GL_COLOR_BUFFER_BIT));

    glUseProgram(shaderProgram);
    glBindVertexArray(vertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexArrayObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);

    glBindTexture(GL_TEXTURE_2D, textureObject);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow(wnd);

    //SDL_Log("onRender : %d", sdlApp->getRunningTime());
}

void OpenGLRenderHelper::createRenderer()
{
    if (wnd == nullptr) return;
    clear();

    glCtx = SDL_GL_CreateContext(wnd);
    SDL_GL_MakeCurrent(wnd, glCtx);

    glewExperimental = true;
    GLenum glRet = glewInit();
    if (glRet != GLEW_OK)
    {
        SDL_Log("Can't init glew.\n");
    }

    int w, h;
    SDL_GetWindowSize(wnd, &w, &h);
    glCall(glViewport(0, 0, w, h));

    SDL_Log("open gl vendor  : %s", (char*)glGetString(GL_VENDOR));
    SDL_Log("open gl version : %s", (char*)glGetString(GL_VERSION));

    createShaderProgram();
    createObjects();
}

void OpenGLRenderHelper::createTexture(int w, int h)
{
    SDL_GL_MakeCurrent(wnd, glCtx);

    glCall(glGenBuffers(1, &bufferObject));
    glCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufferObject));
    glCall(glBufferData(GL_PIXEL_UNPACK_BUFFER, w * h * 4, nullptr, GL_STREAM_DRAW));

    glCall(glGenTextures(1, &textureObject));
    glCall(glBindTexture(GL_TEXTURE_2D, textureObject));
    glCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    glCall(glUseProgram(shaderProgram));
    GLint location = glGetUniformLocation(shaderProgram, "texture1");
    glCheckError();
    glCall(glUniform1i(location, 0));

    glCall(glUseProgram(0));
    glCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
    glCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void OpenGLRenderHelper::clear()
{
    SDL_GL_MakeCurrent(wnd, glCtx);

    if (bufferObject != 0)
    {
        glCall(glDeleteBuffers(1, &bufferObject));
    }

    if (textureObject != 0)
    {
        glCall(glDeleteTextures(1, &textureObject));
    }

    if (glCtx != nullptr)
    {
        SDL_GL_DeleteContext(glCtx);
    }

    bufferObject = 0;
    textureObject = 0;
    glCtx = nullptr;
}

const char* vertexShaderSource = "#version 460 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x,aPos.y,aPos.z,1.0f);\n"
"   TexCoord = aTexCoord;\n"
"}\n";

const char* fragmentShaderSource = "#version 460 core\n"
"out vec4 fragColor;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D texture1;\n"
"void main()\n"
"{\n"
"   fragColor = texture(texture1,TexCoord);\n"
"}\n";

void OpenGLRenderHelper::createShaderProgram()
{
    SDL_GL_MakeCurrent(wnd, glCtx);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glCall(glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr));
    glCall(glCompileShader(vertexShader));

    int ret;
    char errorInfo[1024] = { 0 };
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &ret);
    if (ret == 0)
    {
        glGetShaderInfoLog(vertexShader, 1024, nullptr, errorInfo);
        SDL_Log("can't compile vertex shader:%s.", errorInfo);
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glCall(glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr));
    glCall(glCompileShader(fragmentShader));
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &ret);
    if (ret == 0)
    {
        memset(errorInfo, 0, 1024);
        glGetShaderInfoLog(fragmentShader, 1024, nullptr, errorInfo);
        SDL_Log("can't compile fragment shader:%s.", errorInfo);
    }

    shaderProgram = glCreateProgram();
    glCall(glAttachShader(shaderProgram, vertexShader));
    glCall(glAttachShader(shaderProgram, fragmentShader));
    glCall(glLinkProgram(shaderProgram));
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &ret);
    if (ret == 0)
    {
        memset(errorInfo, 0, 1024);
        glGetProgramInfoLog(shaderProgram, 1024, nullptr, errorInfo);
        SDL_Log("can't link shaders:%s.", errorInfo);
    }
    glCall(glDeleteShader(vertexShader));
    glCall(glDeleteShader(fragmentShader));
}

void OpenGLRenderHelper::createObjects()
{
    SDL_GL_MakeCurrent(wnd, glCtx);

    float vertices[] = {
        1.0f,1.0f,0.0f,    1.0f,0.0f,
        1.0f,-1.0f,0.0f,   1.0f,1.0f,
        -1.0f,-1.0f,0.0f,  0.0f,1.0f,
        -1.0f,1.0f,0.0f,   0.0f,0.0f
    };

    GLuint indices[] = {
        0,1,3,
        1,2,3
    };

    glCall(glGenVertexArrays(1, &vertexArrayObject));
    glCall(glGenBuffers(1, &vertexBuffObject));
    glCall(glGenBuffers(1, &indexBufferObject));

    glCall(glBindVertexArray(vertexArrayObject));

    glCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffObject));
    glCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject));
    glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

    // position attribute
    glCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));
    glCall(glEnableVertexAttribArray(0));

    // texture coord attribute
    glCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
    glCall(glEnableVertexAttribArray(1));

    glCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    glCall(glBindVertexArray(0));
}
