#pragma once

#include <GLES2/gl2.h>
#include <vector>

#include "graphics/Color.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Texture.hpp"
#include "math/Math.hpp"
#include "math/Rect.hpp"

namespace salshalon::graphics {

/// Dynamic 2D quad batcher.
///
/// All quads are accumulated into one vertex buffer and submitted with as few
/// draw calls as possible (one per texture change). Alpha blending is on.
class SpriteBatch {
public:
  static constexpr int kMaxQuadsPerBatch = 4096;

  SpriteBatch() = default;
  ~SpriteBatch();

  SpriteBatch(const SpriteBatch &) = delete;
  SpriteBatch &operator=(const SpriteBatch &) = delete;

  bool init();
  void shutdown();

  void begin(const math::Mat4 &projection);
  void end();

  /// Draws `texture` into `dest` (design-space units, top-left origin).
  /// `source` is an optional sub-rect in texel units; null == whole texture.
  /// `origin` is the rotation pivot, relative to the top-left of `dest`.
  void draw(const Texture &texture, const math::Rect &dest,
            const math::Rect *source = nullptr, const Color &tint = kWhite,
            float rotationRadians = 0.0f,
            const math::Vec2 &origin = {0.0f, 0.0f});

  /// Solid-colour quad (uses the built-in 1x1 white texture).
  void drawRect(const math::Rect &dest, const Color &color);

  const Texture &whiteTexture() const { return this->whiteTexture_; }

private:
  struct Vertex {
    float x, y;
    float u, v;
    float r, g, b, a;
  };

  void flush();

  GLuint vbo_ = 0;
  GLuint ibo_ = 0;
  Shader shader_{};
  Texture whiteTexture_{};
  GLuint currentTexture_ = 0;

  std::vector<Vertex> vertices_{};
  int quadCount_ = 0;
  bool active_ = false;

  GLint attribPosition_ = -1;
  GLint attribTexCoord_ = -1;
  GLint attribColor_ = -1;
};

} // namespace salshalon::graphics
