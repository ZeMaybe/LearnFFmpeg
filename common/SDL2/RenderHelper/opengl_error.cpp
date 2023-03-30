
#include "opengl_error.h"

extern "C"
{
#include "GL/glew.h"
#include "GL/wglew.h"
#include "SDL.h"
}
#include <stdlib.h>


void glClearError()
{
#ifdef NDEBUG
    return;
#endif // NDEBUG

    while (glGetError() != GL_NO_ERROR);
}

void glCheckError()
{
#ifdef NDEBUG
    return;
#endif // NDEBUG

    GLenum error;
    while (error = glGetError())
    {
        SDL_Log("OpenGL error : 0x%X\n", error);
    }
}

void glLogCall(const char* func, const char* file, int line)
{
#ifdef NDEBUG
    return;
#endif // NDEBUG

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
}

