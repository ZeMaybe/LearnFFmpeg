
extern "C"
{
#include "libavcodec/avcodec.h"
}

#include "convert.h"
#include "OpenGLRenderHelper.h"
#include "SDLApp.h"
#include "ColorSpace.h"
#include <chrono>

OpenGLRenderHelper::OpenGLRenderHelper(SDL_Window* w)
    :RenderHelper(w)
{
    createRenderer();
}

OpenGLRenderHelper:: ~OpenGLRenderHelper()
{
    clear();
}

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
    auto  re = cudaGraphicsGLRegisterBuffer(&cudaResource,bufferObject, cudaGraphicsRegisterFlagsWriteDiscard);
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

    auto t1 = std::chrono::high_resolution_clock::now();
    Nv12ToColor32<RGBA32>(frame->buf[0]->data, frame->width, (uint8_t*)cudaPointer, frame->width* 4, frame->width, frame->height,(int)frame->colorspace);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto d1 = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
    //SDL_Log("time for color space change:%lld", d1.count());

    cudaGraphicsUnmapResources(1, &cudaResource, 0);
    cudaGraphicsUnregisterResource(cudaResource);
}

void OpenGLRenderHelper::onRender()
{
    SDL_GL_MakeCurrent(wnd, glCtx);

    int w, h;
    SDL_GetWindowSize(wnd, &w, &h);
    glViewport(0, 0, w, h);

    glClearColor(0.3f, 1.0f, 0.5f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(wnd);

    //SDL_Log("onRender : %d", sdlApp->getRunningTime());
}

void OpenGLRenderHelper::createRenderer()
{
    if (wnd == nullptr) return;
    clear();

    glCtx = SDL_GL_CreateContext(wnd);

    glewExperimental = true;
    GLenum glRet = glewInit();
    if (glRet != GLEW_OK)
    {
        SDL_Log("Can't init glew.\n");
    }

    SDL_Log("open gl vendor  : %s", (char*)glGetString(GL_VENDOR));
    SDL_Log("open gl version : %s", (char*)glGetString(GL_VERSION));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
}

void OpenGLRenderHelper::createTexture(int w,int h)
{
    SDL_GL_MakeCurrent(wnd, glCtx);

    glGenBuffers(1, &bufferObject);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufferObject);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, w * h * 4, nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glGenTextures(1, &textureObject);
    glBindTexture(GL_TEXTURE_RECTANGLE, textureObject);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA8, w, h, 0, GL_RGBA8, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);

    // fragment shader...
    // fragment shader...
    // fragment shader...
    // fragment shader...

}

void OpenGLRenderHelper::clear()
{
    SDL_GL_MakeCurrent(wnd, glCtx);

    if (bufferObject != 0)
    {
        glDeleteBuffers(1, &bufferObject);
    }

    if (textureObject != 0)
    {
        glDeleteTextures(1, &textureObject);
    }

    if (glCtx != nullptr)
    {
        SDL_GL_DeleteContext(glCtx);
    }

    bufferObject = 0;
    textureObject = 0;
    glCtx = nullptr;
}
