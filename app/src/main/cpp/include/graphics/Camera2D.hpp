#pragma once

#include "input/ScreenToWorld.hpp"
#include "math/Math.hpp"
#include "math/Rect.hpp"

namespace salshalon::graphics {

/// Orthographic 2D camera with a fixed "design resolution".
///
/// The projection is set up with a top-left origin and +Y pointing down, so a
/// UI rect of {0, 0, 720, 1280} covers the whole screen and reads naturally.
/// The design space is then fitted (letterboxed) into the real surface.
class Camera2D {
public:
  void setDesignResolution(float width, float height);
  void setSurfaceSize(int pixelWidth, int pixelHeight);

  const math::Mat4 &projection() const { return this->projection_; }
  const input::ScreenToWorld &transform() const { return this->transform_; }

  float designWidth() const { return designWidth_; }
  float designHeight() const { return designHeight_; }
  math::Rect worldBounds() const {
    return {0.0f, 0.0f, designWidth_, designHeight_};
  }

  /// Issues the glViewport() call for the letterboxed region.
  void applyViewport() const;

private:
  void rebuild();

  float designWidth_ = 720.0f;
  float designHeight_ = 1280.0f;
  int surfaceWidth_ = 0;
  int surfaceHeight_ = 0;

  math::Mat4 projection_{1.0f};
  input::ScreenToWorld transform_{};
};

} // namespace salshalon::graphics
