#include "ui/Toast.hpp"

#include <algorithm>
#include <utility>

#include "graphics/Font.hpp"
#include "graphics/Texture.hpp"
#include "math/Math.hpp"
#include "ui/Renderer.hpp"

namespace salshalon::ui {

namespace {

/// One line's pixel height at the given text scale.
float lineHeight(float scale) { return graphics::Font::kGlyphHeight * scale; }

} // namespace

void Toast::show(std::string message, float durationSeconds) {
  this->message_ = std::move(message);
  this->timer_ = durationSeconds > 0.0f ? durationSeconds : 0.001;
}

void Toast::dismiss() {
  this->message_.clear();
  this->title_.clear();
  this->timer_ = 0.0f;
}

void Toast::update(float deltaSeconds) {
  if (this->timer_ > 0.0f) {
    this->timer_ -= deltaSeconds;
    if (this->timer_ <= 0.0f) {
      this->dismiss();
    }
  }
}

Toast &Toast::setKind(ToastKind kind) {
  switch (kind) {
  case ToastKind::Info:
    style_.background = {0.78f, 0.86f, 0.98f, 1.0f};
    style_.border = {0.36f, 0.62f, 0.94f, 1.0f};
    style_.titleColor = {0.08f, 0.14f, 0.28f, 1.0f};
    style_.bodyColor = {0.16f, 0.22f, 0.36f, 1.0f};
    style_.iconTint = {0.20f, 0.44f, 0.78f, 1.0f};
    break;

  case ToastKind::Warning:
    style_.background = {0.99f, 0.94f, 0.78f, 1.0f};
    style_.border = {0.92f, 0.68f, 0.16f, 1.0f};
    style_.titleColor = {0.28f, 0.18f, 0.02f, 1.0f};
    style_.bodyColor = {0.34f, 0.26f, 0.10f, 1.0f};
    style_.iconTint = {0.92f, 0.68f, 0.16f, 1.0f};
    break;

  case ToastKind::Error:
    style_.background = {0.99f, 0.85f, 0.85f, 1.0f};
    style_.border = {0.90f, 0.36f, 0.36f, 1.0f};
    style_.titleColor = {0.30f, 0.06f, 0.06f, 1.0f};
    style_.bodyColor = {0.36f, 0.14f, 0.14f, 1.0f};
    style_.iconTint = {0.90f, 0.36f, 0.36f, 1.0f};
    break;

  case ToastKind::Success:
    style_.background = {0.82f, 0.95f, 0.85f, 1.0f};
    style_.border = {0.34f, 0.72f, 0.42f, 1.0f};
    style_.titleColor = {0.06f, 0.24f, 0.12f, 1.0f};
    style_.bodyColor = {0.12f, 0.30f, 0.18f, 1.0f};
    style_.iconTint = {0.34f, 0.72f, 0.42f, 1.0f};
    break;

  case ToastKind::Custom:
    // Leave everything as-is; caller sets it explicitly.
    break;
  }
  return *this;
}

Toast &Toast::setStyle(Style style) {
  style_ = std::move(style);
  return *this;
}

Toast &Toast::setTitle(std::string title) {
  title_ = std::move(title);
  return *this;
}

Toast &Toast::setIcon(const graphics::Texture *texture) {
  icon_ = texture;
  return *this;
}

Toast &Toast::setFadeTime(float seconds) {
  fadeTime_ = seconds > 0.0f ? seconds : 0.001f;
  return *this;
}

bool Toast::hasIcon() const { return icon_ != nullptr && icon_->valid(); }

float Toast::measureContentHeight() const {
  float height = 0.0f;

  if (!title_.empty()) {
    height += lineHeight(style_.titleScale);
    if (!message_.empty())
      height += style_.titleBodyGap;
  }
  if (!message_.empty()) {
    height += lineHeight(style_.bodyScale);
  }
  return height;
}

void Toast::render(Renderer &renderer, const math::Vec2 &anchor,
                   ToastAnchor anchorMode) const {
  if (!visible())
    return;

  const float alpha = math::clampf(this->timer_ / fadeTime_, 0.0f, 1.0f);

  // ---- Box size -------------------------------------------------------------
  const float contentHeight = measureContentHeight();
  const bool iconPresent = hasIcon();

  // The box must fit both the text column and the icon.
  const float iconHeight = iconPresent ? style_.iconSize.y : 0.0f;
  const float innerHeight = std::max(contentHeight, iconHeight);
  const float boxHeight = innerHeight + style_.padding * 2.0f;
  const float boxWidth = style_.boxWidth;

  // Resolve the anchor into the box's centre.
  math::Vec2 boxCenter = anchor;
  switch (anchorMode) {
  case ToastAnchor::Center:
    break;
  case ToastAnchor::BottomCenter:
    boxCenter.y -= boxHeight * 0.5f;
    break;
  case ToastAnchor::TopCenter:
    boxCenter.y += boxHeight * 0.5f;
    break;
  }

  const math::Rect box = math::Rect::fromCenter(boxCenter, boxWidth, boxHeight);

  // ---- Box fill + border ----------------------------------------------------
  graphics::Color bg = style_.background;
  bg.a *= alpha;
  renderer.drawRect(box, bg);

  graphics::Color border = style_.border;
  border.a *= alpha;
  renderer.drawFrame(box, border, style_.borderThickness);

  // ---- Icon -----------------------------------------------------------------
  float textLeft = box.x + style_.padding;
  if (iconPresent) {
    graphics::Color tint = style_.iconTint;
    tint.a *= alpha;

    const math::Rect iconRect{
        box.x + style_.padding,
        boxCenter.y - style_.iconSize.y * 0.5f,
        style_.iconSize.x,
        style_.iconSize.y,
    };
    renderer.drawSprite(*icon_, iconRect, tint);
    textLeft = iconRect.right() + style_.iconGap;
  }

  // ---- Title + body ---------------------------------------------------------
  // Vertically centre the text column inside the box.
  const float textTop = boxCenter.y - contentHeight * 0.5f;
  float cursorY = textTop;

  if (!title_.empty()) {
    graphics::Color c = style_.titleColor;
    c.a *= alpha;
    renderer.drawText(title_, {textLeft, cursorY}, style_.titleScale, c);

    cursorY += lineHeight(style_.titleScale);
    if (!message_.empty())
      cursorY += style_.titleBodyGap;
  }

  if (!message_.empty()) {
    graphics::Color c = style_.bodyColor;
    c.a *= alpha;
    renderer.drawText(message_, {textLeft, cursorY}, style_.bodyScale, c);
  }
}

} // namespace salshalon::ui
