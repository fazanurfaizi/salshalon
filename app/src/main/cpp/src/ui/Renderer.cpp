#include "ui/Renderer.hpp"

#include "graphics/Font.hpp"
#include "graphics/SpriteBatch.hpp"
#include "graphics/Texture.hpp"

namespace salshalon::ui {

void Renderer::drawRect(const math::Rect &rect, const graphics::Color &color) {
  if (this->batch_ == nullptr)
    return;
  this->batch_->drawRect(rect, color);
}

void Renderer::drawFrame(const math::Rect &rect, const graphics::Color &color,
                         float thickness) {
  if (this->batch_ == nullptr)
    return;

  this->drawRect({rect.x, rect.y, rect.width, thickness}, color); // Top
  this->drawRect({rect.x, rect.bottom() - thickness, rect.width, thickness},
                 color); // Bottom
  this->drawRect(
      {rect.x, rect.y + thickness, thickness, rect.height - thickness * 2.0f},
      color); // Left
  this->drawRect({rect.right() - thickness, rect.y + thickness, thickness,
                  rect.height - thickness * 2.0f},
                 color); // Right
}

void Renderer::drawSprite(const graphics::Texture &texture,
                          const math::Rect &dest, const graphics::Color &tint) {
  if (this->batch_ == nullptr)
    return;
  this->batch_->draw(texture, dest, nullptr, tint);
}

void Renderer::drawText(const std::string &text, const math::Vec2 &position,
                        float scale, const graphics::Color &color) {
  if (this->batch_ == nullptr)
    return;
  this->font_.draw(*this->batch_, text, position, scale, color,
                   /*horizontallyCentered=*/false);
}

void Renderer::drawTextCentered(const std::string &text,
                                const math::Vec2 &center, float scale,
                                const graphics::Color &color) {
  if (this->batch_ == nullptr)
    return;

  const float halfLine = (graphics::Font::kGlyphHeight * scale) * 0.5f;
  this->font_.draw(*this->batch_, text, {center.x, center.y - halfLine}, scale,
                   color, /*horizontallyCentered=*/true);
}

float Renderer::measureText(const std::string &text, float scale) const {
  return this->font_.measureWidth(text, scale);
}

} // namespace salshalon::ui
