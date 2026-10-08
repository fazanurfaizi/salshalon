#pragma once

#include "math/Math.hpp"

namespace salshalon::graphics {

/// Floating-point RGBA. 1.0 == full intensity.
struct Color {
  float r = 1.0f;
  float g = 1.0f;
  float b = 1.0f;
  float a = 1.0f;

  constexpr Color() = default;
  constexpr Color(float r_, float g_, float b_, float a_ = 1.0f)
      : r(r_), g(g_), b(b_), a(a_) {}

  math::Vec4 toVec4() const { return {r, g, b, a}; }

  Color withAlpha(float alpha) const { return {r, g, b, alpha}; }
};

inline constexpr Color kWhite{1.0f, 1.0f, 1.0f, 1.0f};
inline constexpr Color kBlack{0.0f, 0.0f, 0.0f, 1.0f};
inline constexpr Color kTransparent{0.0f, 0.0f, 0.0f, 0.0f};

inline Color lerp(const Color &a, const Color &b, float t) {
  return {math::lerpf(a.r, b.r, t), math::lerpf(a.g, b.g, t),
          math::lerpf(a.b, b.b, t), math::lerpf(a.a, b.a, t)};
}

} // namespace salshalon::graphics
