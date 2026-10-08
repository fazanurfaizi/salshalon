#include "graphics/Camera2D.hpp"

#include <GLES2/gl2.h>
#include <algorithm>
#include <cmath>

namespace salshalon::graphics {

void Camera2D::setDesignResolution(float width, float height) {
  if (width <= 0.0f || height <= 0.0f)
    return;
  this->designWidth_ = width;
  this->designHeight_ = height;
  this->rebuild();
}

void Camera2D::setSurfaceSize(int pixelWidth, int pixelHeight) {
  if (pixelWidth == this->surfaceWidth_ && pixelHeight == this->surfaceHeight_)
    return;
  this->surfaceWidth_ = pixelWidth;
  this->surfaceHeight_ = pixelHeight;
  this->rebuild();
}

void Camera2D::rebuild() {
  // Top-left origin, +Y down: ortho(left, right, bottom, top, near, far).
  this->projection_ = glm::ortho(0.0f, this->designWidth_, this->designHeight_,
                                 0.0f, -1.0f, 1.0f);
  if (this->surfaceWidth_ <= 0 || this->surfaceHeight_ <= 0) {
    this->transform_ = {};
    return;
  }

  // "Fit" (letterbox): preserve aspect ratio.
  const float scaleX = static_cast<float>(surfaceWidth_) / designWidth_;
  const float scaleY = static_cast<float>(surfaceHeight_) / designHeight_;
  const float scale = std::min(scaleX, scaleY);

  transform_.scale = scale;
  transform_.offsetX =
      (static_cast<float>(surfaceWidth_) - designWidth_ * scale) * 0.5f;
  transform_.offsetY =
      (static_cast<float>(surfaceHeight_) - designHeight_ * scale) * 0.5f;
}

void Camera2D::applyViewport() const {
  const int viewportWidth =
      static_cast<int>(std::lround(designWidth_ * transform_.scale));
  const int viewportHeight =
      static_cast<int>(std::lround(designHeight_ * transform_.scale));

  if (viewportWidth <= 0 || viewportHeight <= 0) {
    glViewport(0, 0, surfaceWidth_, surfaceHeight_);
    return;
  }

  // glViewport's origin is bottom-left; letterbox is centred, so the
  // mirrored offset is identical and we can reuse offsetY directly.
  glViewport(static_cast<int>(transform_.offsetX),
             static_cast<int>(transform_.offsetY), viewportWidth,
             viewportHeight);
}

} // namespace salshalon::graphics
