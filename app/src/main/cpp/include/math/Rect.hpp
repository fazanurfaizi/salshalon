#pragma once

#include "math/Math.hpp"

namespace salshalon::math {

/// Axis-aligned bounding box, used for both UI hit-testing and sprite layout.
/// Coordinates are top-left origin with +Y pointing down (matches the camera).
struct Rect {
  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;

  constexpr Rect() = default;
  constexpr Rect(float x_, float y_, float width_, float height_)
      : x(x_), y(y_), width(width_), height(height_) {}

  static Rect fromCenter(const Vec2 &center, float w, float h) {
    return {center.x - w * 0.5f, center.y - h * 0.5f, w, h};
  }

  float left() const { return this->x; }
  float top() const { return this->y; }
  float right() const { return this->x + this->width; }
  float bottom() const { return this->y + this->height; }

  Vec2 center() const { return {x + width * 0.5f, y + height * 0.5f}; }
  Vec2 size() const { return {width, height}; }

  /// AABB point containment
  bool contains(const Vec2 &p) const;

  /// AABB-vs-AABB overlap
  bool intersects(const Rect &other) const;

  Rect inset(float amount) const {
    return {x + amount, y + amount, width - amount * 2.0f,
            height - amount * 2.0f};
  }

  Rect expanded(float amount) const { return inset(-amount); }
};

} // namespace salshalon::math
