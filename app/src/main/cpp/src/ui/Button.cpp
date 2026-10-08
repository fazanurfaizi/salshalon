#include "ui/Button.hpp"

#include <utility>

#include "core/Log.hpp"
#include "input/TouchEvent.hpp"
#include "math/Math.hpp"
#include "ui/Renderer.hpp"

namespace salshalon::ui {

Button::Button(std::string label, const math::Rect &bounds, Callback onClick)
    : label_(std::move(label)), onClick_(std::move(onClick)) {
  this->bounds = bounds;
}

void Button::setColors(const graphics::Color &idle,
                       const graphics::Color &pressed,
                       const graphics::Color &text) {
  this->idleColor_ = idle;
  this->pressedColor_ = pressed;
  this->textColor_ = text;
}

void Button::update(float deltaSeconds) {
  const float target = this->pressed_ ? 1.0f : 0.0f;
  const float speed = math::clampf(deltaSeconds * 18.0f, 0.0f, 1.0f);
  this->pressAnim_ += (target - this->pressAnim_) * speed;
}

void Button::render(Renderer &ui) {
  if (!visible)
    return;

  const float shrink = this->pressAnim_ * 3.0f;
  const math::Rect box = bounds.inset(shrink);

  const graphics::Color fill =
      graphics::lerp(this->idleColor_, this->pressedColor_, this->pressAnim_);
  const graphics::Color border = graphics::lerp(
      this->borderColor_, graphics::kWhite.withAlpha(0.9f), this->pressAnim_);

  ui.drawRect(box, fill);
  ui.drawFrame(box, border, 3.0f);

  if (!this->label_.empty()) {
    ui.drawTextCentered(this->label_, box.center(), this->textScale_,
                        enabled ? this->textColor_
                                : this->textColor_.withAlpha(0.4f));
  }
}

bool Button::onTouch(const input::TouchEvent &event) {
  if (!this->visible || !this->enabled)
    return false;

  switch (event.phase) {
  case input::TouchPhase::Down:
    if (this->bounds.contains(event.position)) {
      this->pressed_ = true;
      return true;
    }
    return false;

  case input::TouchPhase::Move:
    // Dragging off the button cancels the click.
    if (this->pressed_ && !this->bounds.contains(event.position))
      this->pressed_ = false;
    return this->pressed_;

  case input::TouchPhase::Up:
    if (this->pressed_) {
      this->pressed_ = false;
      if (this->bounds.contains(event.position)) {
        if (this->onClick_)
          this->onClick_();
        return true;
      }
    }
    return false;

  case input::TouchPhase::Cancel:
    this->pressed_ = false;
    return false;
  }
  return false;
}

} // namespace salshalon::ui
