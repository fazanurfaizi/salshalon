#pragma once

#include <EGL/egl.h>

struct ANativeWindow;

namespace salshalon::core {

class GLContext {
public:
  GLContext() = default;
  ~GLContext();

  GLContext(const GLContext &) = delete;
  GLContext &operator=(const GLContext &) = delete;

  bool init(ANativeWindow *window);
  void destroy();

  bool isValid() const { return surface_ != EGL_NO_SURFACE; }
  void swapBuffers() const;

  /// Re-reads the surface dimensions
  void querySize();

  int width() const { return width_; }
  int height() const { return height_; }

private:
  EGLDisplay display_ = EGL_NO_DISPLAY;
  EGLSurface surface_ = EGL_NO_SURFACE;
  EGLContext context_ = EGL_NO_CONTEXT;
  EGLConfig config_ = nullptr;
  int width_ = 0;
  int height_ = 0;
};

} // namespace salshalon::core
