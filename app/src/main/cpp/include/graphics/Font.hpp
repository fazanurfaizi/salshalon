#pragma once

#include <string>

#include "graphics/Color.hpp"
#include "math/Math.hpp"

namespace salshalon::graphics {

class SpriteBatch;

class Font {
public:
  static constexpr float kGlyphHeight = 8.0f;

  void draw(SpriteBatch &batch, const std::string &text,
            const math::Vec2 &position, float scale,
            const Color &color = kWhite,
            bool horizontallyCentered = false) const;

  float measureWidth(const std::string &text, float scale) const;
  float lineHeight(float scale) const { return kGlyphHeight * scale; }
};

} // namespace salshalon::graphics
