#pragma once

#include <string>

#include "graphics/Color.hpp"
#include "graphics/Font.hpp"
#include "math/Math.hpp"
#include "math/Rect.hpp"

namespace salshalon::graphics {

class SpriteBatch;
class Texture;

} // namespace salshalon::graphics

namespace salshalon::ui {

class Renderer {
public:
  void setBatch(graphics::SpriteBatch *batch) { this->batch_ = batch; }

  void drawRect(const math::Rect &rect, const graphics::Color &color);
  void drawFrame(const math::Rect &rect, const graphics::Color &color,
                 float thickness);

  void drawSprite(const graphics::Texture &texture, const math::Rect &dest,
                  const graphics::Color &tint = graphics::kWhite);

  /// `position` is the top-left of the text box.
  void drawText(const std::string &text, const math::Vec2 &position,
                float scale, const graphics::Color &color = graphics::kWhite);

  /// Centres the text on both axes around `center`.
  void drawTextCentered(const std::string &text, const math::Vec2 &center,
                        float scale,
                        const graphics::Color &color = graphics::kWhite);

  float measureText(const std::string &text, float scale) const;

private:
  graphics::SpriteBatch *batch_ = nullptr;
  graphics::Font font_{};
};

} // namespace salshalon::ui
