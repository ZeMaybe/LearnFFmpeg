
#pragma once
#include "RenderHelper.h"

// use opengl

extern "C"
{
#include "GL/glew.h"
#include "GL/wglew.h"
#include "SDL.h"
//#include "SDL_opengl.h"
}
#include "cuda.h"
#include "cuda_runtime.h"
#include "cuda_gl_interop.h"
//#include "driver_types.h"

class OpenGLRenderHelper : public RenderHelper
{
public:
    OpenGLRenderHelper(SDL_Window*);
    virtual ~OpenGLRenderHelper();

    virtual void update(struct AVFrame* frame) override;
    virtual void onRender() override;

protected:
    SDL_GLContext glCtx = nullptr;

    GLuint bufferObject = 0;
    GLuint textureObject = 0;
    GLuint shaderObject = 0;

    void createRenderer();
    void createTexture(int w,int h);
    void clear();
};
