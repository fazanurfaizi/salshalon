#pragma once

#include "math/Math.hpp"

namespace salshalon::input {

struct ScreenToWorld {
  float scale = 1.0f;
  float offsetX = 0.0f;
  float offsetY = 0.0f;

  math::Vec2 toWorld(const math::Vec2 &screenPixels) const {
    return {(screenPixels.x - offsetX) / scale,
            (screenPixels.y - offsetY) / scale};
  }

  math::Vec2 toScreen(const math::Vec2 &world) const {
    return {world.x * scale + offsetX, world.y * scale + offsetY};
  }
};

} // namespace salshalon::input
