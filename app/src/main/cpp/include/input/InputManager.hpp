#pragma once

#include <vector>

#include "input/ScreenToWorld.hpp"
#include "input/TouchEvent.hpp"

namespace salshalon::input {

/// Collects raw OS touch samples, converts them into design space, and exposes
/// them both as an event list (for widgets) and as persistent state (for hover
/// / drag queries).
class InputManager {
public:
  void setTransform(const ScreenToWorld &transform) { transform_ = transform; }
  const ScreenToWorld &transform() const { return transform_; }

  /// Clears per-frame data. Call once at the top of every frame.
  void beginFrame();

  /// Feeds one raw OS sample (surface pixels). Converts + filters internally.
  void pushRawEvent(const TouchEvent &rawEvent);

  bool isDown() const { return down_; }
  bool justPressed() const { return justPressed_; }
  bool justReleased() const { return justReleased_; }

  /// Current primary pointer position, in design-space units.
  const math::Vec2 &position() const { return worldPosition_; }

  /// All accepted events for this frame, already in design space.
  const std::vector<TouchEvent> &events() const { return events_; }

private:
  ScreenToWorld transform_{};
  std::vector<TouchEvent> events_{};

  math::Vec2 worldPosition_{0.0f, 0.0f};
  int activePointer_ = -1;
  bool down_ = false;
  bool justPressed_ = false;
  bool justReleased_ = false;
};

} // namespace salshalon::input
