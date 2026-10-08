#include "core/GLContext.hpp"

#include <android/native_window.h>

#include "core/Log.hpp"

namespace salshalon::core {

namespace {
// clang-format off
  constexpr EGLint kConfigAttribs [] = {
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
    EGL_RED_SIZE,        8,
    EGL_GREEN_SIZE,      8,
    EGL_BLUE_SIZE,       8,
    EGL_ALPHA_SIZE,      8,
    EGL_DEPTH_SIZE,      0,   // pure 2D: no depth buffer needed
    EGL_STENCIL_SIZE,    0,
    EGL_NONE
  };
// clang-format on

constexpr EGLint kContextAttribs[] = {
    EGL_CONTEXT_CLIENT_VERSION,
    2,
    EGL_NONE,
};
} // namespace

GLContext::~GLContext() { this->destroy(); }

bool GLContext::init(ANativeWindow *window) {
  if (window == nullptr)
    return false;
  if (this->surface_ != EGL_NO_SURFACE)
    return true;

  this->display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (this->display_ == EGL_NO_DISPLAY) {
    SAL_LOGE("EGL: eglGetDisplay failed");
    return false;
  }

  EGLint major = 0;
  EGLint minor = 0;
  if (eglInitialize(this->display_, &major, &minor) != EGL_TRUE) {
    SAL_LOGE("EGL: eglInitialize failed (0x%x)", eglGetError());
    this->display_ = EGL_NO_DISPLAY;
    return false;
  }

  EGLint numConfigs = 0;
  if (eglChooseConfig(this->display_, kConfigAttribs, &this->config_, 1,
                      &numConfigs) != EGL_TRUE ||
      numConfigs < 1) {
    SAL_LOGE("EGL: eglChooseConfig failed (0x%x)", eglGetError());
    this->destroy();
    return false;
  }

  this->surface_ =
      eglCreateWindowSurface(this->display_, this->config_, window, nullptr);
  if (this->surface_ == EGL_NO_SURFACE) {
    SAL_LOGE("EGL: eglCreateWindowSurface failed (0x%x)", eglGetError());
    this->destroy();
    return false;
  }

  this->context_ = eglCreateContext(this->display_, this->config_,
                                    EGL_NO_CONTEXT, kContextAttribs);
  if (this->context_ == EGL_NO_CONTEXT) {
    SAL_LOGE("EGL: eglCreateContext failed (0x%x)", eglGetError());
    this->destroy();
    return false;
  }

  if (eglMakeCurrent(this->display_, this->surface_, this->surface_,
                     this->context_) != EGL_TRUE) {
    SAL_LOGE("EGL: eglMakeCurrent failed (0x%x)", eglGetError());
    this->destroy();
    return false;
  }

  this->querySize();
  SAL_LOGI("EGL %d.%d ready — surface %dx%d", major, minor, this->width_,
           this->height_);
  return true;
}

void GLContext::destroy() {
  if (this->display_ == EGL_NO_DISPLAY)
    return;

  eglMakeCurrent(this->display_, EGL_NO_SURFACE, EGL_NO_SURFACE,
                 EGL_NO_CONTEXT);
  if (this->context_ != EGL_NO_CONTEXT)
    eglDestroyContext(this->display_, this->context_);
  if (this->surface_ != EGL_NO_SURFACE)
    eglDestroySurface(this->display_, this->surface_);
  eglTerminate(this->display_);

  display_ = EGL_NO_DISPLAY;
  surface_ = EGL_NO_SURFACE;
  context_ = EGL_NO_CONTEXT;
  config_ = nullptr;
  width_ = 0;
  height_ = 0;
}

void GLContext::swapBuffers() const {
  if (this->display_ == EGL_NO_DISPLAY || this->surface_ == EGL_NO_SURFACE)
    return;
  eglSwapBuffers(this->display_, this->surface_);
}

void GLContext::querySize() {
  if (this->display_ == EGL_NO_DISPLAY || this->surface_ == EGL_NO_SURFACE)
    return;
  EGLint w = 0;
  EGLint h = 0;
  eglQuerySurface(this->display_, this->surface_, EGL_WIDTH, &w);
  eglQuerySurface(this->display_, this->surface_, EGL_HEIGHT, &h);
  this->width_ = w;
  this->height_ = h;
}

} // namespace salshalon::core
