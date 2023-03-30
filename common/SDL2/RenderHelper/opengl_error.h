#pragma once

/*
* GL_NO_ERROR          : 0x0000
* GL_INVALID_ENUM      : 0x0500
* GL_INVALID_VALUE     : 0x0501
* GL_INVALID_OPERATION : 0x0502
* GL_STACK_OVERFLOW    : 0x503
* GL_STACK_UNDERFLOW   : 0x504
* GL_OUT_OF_MEMORY     : 0x505
* GL_INVALID_FRAMEBUFFER_OPERATION : 0x506
* GL_CONTEXT_LOST      : 0x507
* GL_TABLE_TOO_LARGE   : 0x8301
*/

void glClearError();
void glCheckError();
void glLogCall(const char* func, const char* file, int line);

#define glCall(x) glClearError();x;glLogCall(#x,__FILE__,__LINE__);
