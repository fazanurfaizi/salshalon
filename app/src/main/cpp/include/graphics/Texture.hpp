#pragma once

#include <GLES2/gl2.h>
#include <cstddef>
#include <cstdint>
#include <string>

#include "graphics/Color.hpp"

namespace salshalon::core {
class AssetManager;
}

namespace salshalon::graphics {

/// Move-only 2D texture. RGBA8, LINEAR filtering, CLAMP_TO_EDGE wrapping.
class Texture {
public:
  Texture() = default;
  ~Texture();

  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;
  Texture(Texture &&other) noexcept;
  Texture &operator=(Texture &&other) noexcept;

  /// Decodes a PNG/JPG from the APK assets via stb_image.
  bool loadFromAsset(const core::AssetManager &assets, const std::string &path);

  /// Decodes an in-memory encoded image.
  bool loadFromMemory(const uint8_t *data, std::size_t size,
                      bool flipVertically = false);

  /// 1x1 style flat color
  bool createSolid(int width, int height, const Color &color);

  /// Vertical two-color ramp
  bool createGradient(int width, int height, const Color &top,
                      const Color &bottom);

  void release();

  GLuint id() const { return this->id_; }
  int width() const { return this->width_; }
  int height() const { return this->height_; }
  bool valid() const { return this->id_ != 0; }

private:
  bool upload(const void *pixels, int width, int height,
              GLenum format = GL_RGBA);

  GLuint id_ = 0;
  int width_ = 0;
  int height_ = 0;
};

} // namespace salshalon::graphics
