#include "ui/Dialog.hpp"

#include "input/TouchEvent.hpp"
#include "math/Math.hpp"
#include "ui/Renderer.hpp"

namespace salshalon::ui {

namespace {
float easeOutCubic(float t) {
  const float u = 1.0f - math::clampf(t, 0.0f, 1.0f);
  return 1.0f - u * u * u;
}
} // namespace

void Dialog::show() { this->visible_ = true; }

void Dialog::hide() { this->visible_ = false; }

void Dialog::update(float deltaSeconds) {
  const float target = this->visible_ ? 1.0f : 0.0f;
  const float speed = math::clampf(deltaSeconds * 14.0f, 0.0f, 1.0f);
  this->openAnim_ += (target - this->openAnim_) * speed;

  if (this->openAnim_ < 0.001f)
    this->openAnim_ = 0.0f;
}

bool Dialog::onTouch(const input::TouchEvent &event) {
  if (!this->rendering())
    return false;

  if (this->dismissOnTap_ && this->openAnim_ > 0.95f &&
      event.phase == input::TouchPhase::Up) {
    this->hide();
    if (this->onDismiss_)
      this->onDismiss_();
  }

  return true;
}

void Dialog::render(Renderer &renderer, const math::Rect &viewport) const {
  if (!this->rendering())
    return;

  const float eased = easeOutCubic(this->openAnim_);

  // Backdrop
  graphics::Color backdrop = this->style_.backdrop;
  backdrop.a *= this->openAnim_;
  renderer.drawRect(viewport, backdrop);

  // Panel
  const float slideY = math::lerpf(24.0f, 0.0f, eased);
  const math::Vec2 viewpointCenter = viewport.center();
  const math::Rect panel =
      math::Rect::fromCenter({viewpointCenter.x, viewpointCenter.y},
                             this->style_.panelWidth, this->style_.panelHeight);

  graphics::Color panelFill = this->style_.panel;
  panelFill.a *= this->openAnim_;
  renderer.drawRect(panel, panelFill);

  graphics::Color panelBorder = this->style_.border;
  panelBorder.a *= this->openAnim_;
  renderer.drawFrame(panel, panelBorder, this->style_.borderThickness);

  // Text
  if (!this->title_.empty()) {
    graphics::Color c = this->style_.titleColor;
    c.a *= this->openAnim_;
    renderer.drawTextCentered(
        this->title_, {panel.center().x, panel.y + this->style_.titleTopInset},
        this->style_.titleScale, c);
  }

  if (!this->body_.empty()) {
    graphics::Color c = this->style_.bodyColor;
    c.a *= this->openAnim_;
    renderer.drawTextCentered(this->body_, panel.center(),
                              this->style_.bodyScale, c);
  }

  if (!this->hint_.empty()) {
    graphics::Color c = this->style_.hintColor;
    c.a *= this->openAnim_;
    renderer.drawTextCentered(
        this->hint_,
        {panel.center().x, panel.bottom() - this->style_.hintBottomInset},
        this->style_.hintScale, c);
  }
}

} // namespace salshalon::ui
