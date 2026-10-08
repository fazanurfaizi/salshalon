#pragma once

#include <functional>
#include <string>

#include "graphics/Color.hpp"
#include "ui/Widget.hpp"

namespace salshalon::ui {

class Button : public Widget {
public:
  using Callback = std::function<void()>;

  Button() = default;
  Button(std::string label, const math::Rect &bounds, Callback onClick);

  void setLabel(std::string label) { label_ = std::move(label); }
  const std::string &label() const { return label_; }

  void setOnClick(Callback callback) { onClick_ = std::move(callback); }

  void setColors(const graphics::Color &idle, const graphics::Color &pressed,
                 const graphics::Color &text);

  void setTextScale(float scale) { textScale_ = scale; }

  void update(float deltaSeconds) override;
  void render(Renderer &ui) override;
  bool onTouch(const input::TouchEvent &event) override;

  bool isPressed() const { return pressed_; }

private:
  std::string label_{};
  Callback onClick_{};

  graphics::Color idleColor_{0.13f, 0.16f, 0.24f, 0.96f};
  graphics::Color pressedColor_{0.33f, 0.56f, 0.86f, 1.0f};
  graphics::Color borderColor_{1.0f, 1.0f, 1.0f, 0.22f};
  graphics::Color textColor_{1.0f, 1.0f, 1.0f, 1.0f};

  float textScale_ = 4.0f;
  float pressAnim_ = 0.0f; // 0 = idle, 1 = fully pressed
  bool pressed_ = false;   // finger currently down inside the button
};

} // namespace salshalon::ui
