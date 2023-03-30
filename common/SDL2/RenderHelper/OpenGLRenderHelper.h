
#pragma once
#include "RenderHelper.h"

extern "C"
{
#include "GL/glew.h"
#include "GL/wglew.h"
#include "SDL.h"
}
#include "cuda.h"
#include "cuda_runtime.h"
#include "cuda_gl_interop.h"


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

    void createRenderer();
    void createTexture(int w, int h);
    void clear();

protected:
    GLuint shaderProgram = 0;
    GLuint vertexArrayObject = 0;
    GLuint vertexBuffObject = 0;
    GLuint indexBufferObject = 0;
    void createShaderProgram();
    void createObjects();
};
