#pragma once

#include "math/Rect.hpp"

namespace salshalon::input {
struct TouchEvent;
}

namespace salshalon::ui {

class Renderer;

class Widget {
public:
  virtual ~Widget() = default;

  math::Rect bounds{};
  bool visible = true;
  bool enabled = true;

  virtual void update(float deltaSeconds) { (void)deltaSeconds; }

  virtual void render(Renderer &ui) { (void)ui; }

  virtual bool onTouch(const input::TouchEvent &event) {
    (void)event;
    return false;
  }
};

} // namespace salshalon::ui
