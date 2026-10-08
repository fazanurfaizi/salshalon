#include "graphics/Font.hpp"

#include <algorithm>
#include <vector>

#include "graphics/SpriteBatch.hpp"
#include "math/Rect.hpp"

#define STB_EASY_FONT_IMPLEMENTATION
#include "vendor/stb/stb_easy_font.h"

namespace salshalon::graphics {

namespace {

/// Reused scratch buffer so text drawing performs no per-frame allocation.
std::vector<unsigned char> &scratchBuffer() {
  static std::vector<unsigned char> buffer;
  return buffer;
}

std::vector<char> toMutableCString(const std::string &text) {
  std::vector<char> buffer(text.begin(), text.end());
  buffer.push_back('\0');
  return buffer;
}

} // namespace

float Font::measureWidth(const std::string &text, float scale) const {
  if (text.empty())
    return 0.0f;
  std::vector<char> cstr = toMutableCString(text);
  return static_cast<float>(stb_easy_font_width(cstr.data())) * scale;
}

void Font::draw(SpriteBatch &batch, const std::string &text,
                const math::Vec2 &position, float scale, const Color &color,
                bool horizontallyCentered) const {
  if (text.empty() || scale <= 0.0f)
    return;

  std::vector<char> cstr = toMutableCString(text);

  const int quadCapacity = static_cast<int>(text.size()) * 16 + 8;
  auto &buffer = scratchBuffer();
  buffer.resize(static_cast<std::size_t>(quadCapacity) * 16 * sizeof(float));

  const unsigned char white[4] = {255, 255, 255, 255};
  const int quadCount = stb_easy_font_print(
      0.0f, 0.0f, cstr.data(), const_cast<unsigned char *>(white),
      buffer.data(), static_cast<int>(buffer.size()));

  if (quadCount <= 0)
    return;

  float originX = position.x;
  if (horizontallyCentered) {
    originX -=
        static_cast<float>(stb_easy_font_width(cstr.data())) * scale * 0.5f;
  }

  const float *vertices = reinterpret_cast<const float *>(buffer.data());

  for (int quad = 0; quad < quadCount; ++quad) {
    const float *v = vertices + static_cast<std::size_t>(quad) * 16;

    const float minX = std::min(std::min(v[0], v[4]), std::min(v[8], v[12]));
    const float maxX = std::max(std::min(v[0], v[4]), std::max(v[8], v[12]));
    const float minY = std::min(std::min(v[1], v[5]), std::min(v[9], v[13]));
    const float maxY = std::max(std::min(v[1], v[5]), std::max(v[9], v[13]));

    if (maxX <= minX || maxY <= minY)
      continue;

    const math::Rect dest{originX + minX * scale, position.y + minY * scale,
                          (maxX - minX) * scale, (maxY - minY) * scale};
    batch.drawRect(dest, color);
  }
}

} // namespace salshalon::graphics
