#pragma once

#include "core/Log.hpp"
#include <GLES2/gl2.h>

#ifndef NDEBUG
#define GL_CHECK(expr)                                                         \
  do {                                                                         \
    (expr);                                                                    \
    for (GLenum err = glGetError(); err != GL_NO_ERROR; err = glGetError()) {  \
      SAL_LOGE("GL 0x%04x at %s:%d - %s", err, __FILE__, __LINE__, #expr);     \
    }                                                                          \
  } while (0)
#else
#defiine GL_CHECK(expr)(expr)
#endif // !NDEBUG
