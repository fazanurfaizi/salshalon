#include "graphics/Texture.hpp"

#include <GLES2/gl2.h>
#include <vector>

#include "core/AssetManager.hpp"
#include "core/Log.hpp"
#include "math/Math.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "vendor/stb/stb_image.h"

namespace salshalon::graphics {

namespace {

uint8_t toByte(float value) {
  return static_cast<uint8_t>(math::clampf(value, 0.0f, 1.0f) * 255.0f + 0.5f);
}

} // namespace

Texture::~Texture() { this->release(); }

Texture::Texture(Texture &&other) noexcept
    : id_(other.id_), width_(other.width_), height_(other.height_) {
  other.id_ = 0;
  other.width_ = 0;
  other.height_ = 0;
}

Texture &Texture::operator=(Texture &&other) noexcept {
  if (this != &other) {
    this->release();
    this->id_ = other.id_;
    this->width_ = other.width_;
    this->height_ = other.height_;
    other.id_ = 0;
    other.width_ = 0;
    other.height_ = 0;
  }
  return *this;
}

bool Texture::loadFromAsset(const core::AssetManager &assets,
                            const std::string &path) {
  const std::vector<uint8_t> bytes = assets.read(path.c_str());
  if (bytes.empty())
    return false;
  return this->loadFromMemory(bytes.data(), bytes.size());
}

bool Texture::loadFromMemory(const uint8_t *data, std::size_t size,
                             bool flipVertically) {
  if (data == nullptr || size == 0)
    return false;

  int width = 0;
  int height = 0;
  int channels = 0;

  stbi_set_flip_vertically_on_load_thread(flipVertically ? 1 : 0);
  unsigned char *pixels = stbi_load_from_memory(data, static_cast<int>(size),
                                                &width, &height, &channels, 4);

  if (pixels == nullptr) {
    SAL_LOGE("Texture: decode failed (%s)", stbi_failure_reason());
    return false;
  }

  const bool ok = this->upload(pixels, width, height);
  stbi_image_free(pixels);
  return ok;
}

bool Texture::createSolid(int width, int height, const Color &color) {
  if (width <= 0 || height <= 0)
    return false;

  const uint8_t r = toByte(color.r);
  const uint8_t g = toByte(color.g);
  const uint8_t b = toByte(color.b);
  const uint8_t a = toByte(color.a);

  std::vector<uint8_t> pixels(static_cast<std::size_t>(width) * height * 4);
  for (std::size_t i = 0; i < pixels.size(); i += 4) {
    pixels[i + 0] = r;
    pixels[i + 1] = g;
    pixels[i + 2] = b;
    pixels[i + 3] = a;
  }
  return this->upload(pixels.data(), width, height);
}

bool Texture::createGradient(int width, int height, const Color &top,
                             const Color &bottom) {
  if (width <= 0 || height <= 0)
    return false;

  std::vector<uint8_t> pixels(static_cast<std::size_t>(width) * height * 4);
  for (int y = 0; y < height; ++y) {
    const float t = (height > 1)
                        ? static_cast<float>(y) / static_cast<float>(height - 1)
                        : 0.0f;
    const Color row = lerp(top, bottom, t);
    for (int x = 0; x < width; ++x) {
      uint8_t *p = &pixels[(static_cast<std::size_t>(y) * width + x) * 4];
      p[0] = toByte(row.r);
      p[1] = toByte(row.g);
      p[2] = toByte(row.b);
      p[3] = toByte(row.a);
    }
  }
  return this->upload(pixels.data(), width, height);
}

bool Texture::upload(const void *pixels, int width, int height, GLenum format) {
  if (pixels == nullptr || width <= 0 || height <= 0)
    return false;
  this->release();

  GLuint id = 0;
  glGenTextures(1, &id);
  if (id == 0) {
    SAL_LOGE("Texture: glGenTextures failed");
    return false;
  }

  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, pixels);
  glBindTexture(GL_TEXTURE_2D, 0);

  this->id_ = id;
  this->width_ = width;
  this->height_ = height;
  return true;
}

void Texture::release() {
  if (this->id_ != 0) {
    glDeleteTextures(1, &this->id_);
    this->id_ = 0;
  }
  this->width_ = 0;
  this->height_ = 0;
}

} // namespace salshalon::graphics
