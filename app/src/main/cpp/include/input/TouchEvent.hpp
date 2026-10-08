#pragma once

#include <cstdint>

#include "math/Math.hpp"

namespace salshalon::input {

enum class TouchPhase { Down, Move, Up, Cancel };

struct TouchEvent {
  int pointerId = 0;
  TouchPhase phase = TouchPhase::Down;
  math::Vec2 position{0.0f, 0.0f};
  int64_t timeMs = 0;
};

} // namespace salshalon::input
