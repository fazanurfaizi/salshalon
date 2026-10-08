#include "graphics/SpriteBatch.hpp"

#include <GLES2/gl2.h>
#include <cmath>
#include <cstddef>

#include "core/Log.hpp"
#include "graphics/Color.hpp"
#include "graphics/GLCheck.hpp"

namespace salshalon::graphics {

namespace {

constexpr const char *kSpriteVertexShader = R"(#version 100
uniform mat4 u_Projection;

attribute vec2 a_Position;
attribute vec2 a_TexCoord;
attribute vec4 a_Color;

varying vec2 v_TexCoord;
varying vec4 v_Color;

void main() {
  v_TexCoord = a_TexCoord;
  v_Color = a_Color;
  gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
}
)";

constexpr const char *kSpriteFragmentShader = R"(#version 100
precision mediump float;

uniform sampler2D u_Texture;

varying vec2 v_TexCoord;
varying vec4 v_Color;

void main() {
  gl_FragColor = texture2D(u_Texture, v_TexCoord) * v_Color;
}
)";

} // namespace

SpriteBatch::~SpriteBatch() { this->shutdown(); }

bool SpriteBatch::init() {
  if (this->vbo_ != 0)
    return true;

  if (!this->shader_.buildFromSource(kSpriteVertexShader,
                                     kSpriteFragmentShader)) {
    SAL_LOGE("SpriteBatch: shader build failed");
    return false;
  }

  this->attribPosition_ = glGetAttribLocation(this->shader_.id(), "a_Position");
  this->attribTexCoord_ = glGetAttribLocation(this->shader_.id(), "a_TexCoord");
  this->attribColor_ = glGetAttribLocation(this->shader_.id(), "a_Color");

  if (this->attribPosition_ < 0 || attribTexCoord_ < 0 || attribColor_ < 0) {
    SAL_LOGE("SpriteBatch: missing shader attributes");
    return false;
  }

  this->vertices_.resize(static_cast<std::size_t>(kMaxQuadsPerBatch) * 4);

  glGenBuffers(1, &this->vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo_);
  glBufferData(GL_ARRAY_BUFFER, this->vertices_.size() * sizeof(Vertex),
               nullptr, GL_DYNAMIC_DRAW);

  // Pre-baked quad indices: 0,1,2  0,2,3 for every quad slot.
  std::vector<GLushort> indices(static_cast<std::size_t>(kMaxQuadsPerBatch) *
                                6);
  for (int quad = 0; quad < kMaxQuadsPerBatch; ++quad) {
    const GLushort base = static_cast<GLushort>(quad * 4);
    GLushort *dst = &indices[static_cast<GLushort>(quad * 6)];
    dst[0] = base + 0;
    dst[1] = base + 1;
    dst[2] = base + 2;
    dst[3] = base + 0;
    dst[4] = base + 2;
    dst[5] = base + 3;
  }

  glGenBuffers(1, &this->ibo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort),
               indices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  if (!this->whiteTexture_.createSolid(1, 1, kWhite)) {
    SAL_LOGE("SpriteBatch: white texture creation failed");
    return false;
  }

  return true;
}

void SpriteBatch::shutdown() {
  if (this->vbo_ != 0) {
    glDeleteBuffers(1, &this->vbo_);
    this->vbo_ = 0;
  }
  if (this->ibo_ != 0) {
    glDeleteBuffers(1, &this->ibo_);
    this->ibo_ = 0;
  }
  this->shader_.destroy();
  this->whiteTexture_.release();
  this->vertices_.clear();
  this->quadCount_ = 0;
  this->active_ = false;
}

void SpriteBatch::begin(const math::Mat4 &projection) {
  if (this->vbo_ == 0)
    return;

  this->quadCount_ = 0;
  this->currentTexture_ = 0;
  this->active_ = true;

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  this->shader_.bind();
  this->shader_.setMat4("u_Projection", projection);
  this->shader_.setInt("u_Texture", 0);

  glActiveTexture(GL_TEXTURE0);
}

void SpriteBatch::end() {
  if (!this->active_)
    return;
  this->flush();
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
  this->active_ = false;
}

// clang-format off
void SpriteBatch::draw(
  const Texture &texture,
  const math::Rect &dest,
  const math::Rect *source,
  const Color &tint,
  float rotationRadians,
  const math::Vec2 &origin) {
  // clang-format on

  if (!this->active_ || !texture.valid())
    return;

  // Texture change or full batch
  if (this->currentTexture_ != texture.id() ||
      this->quadCount_ >= kMaxQuadsPerBatch) {
    this->flush();
    this->currentTexture_ = texture.id();
  }

  float u0 = 0.0f;
  float u1 = 1.0f;
  float v0 = 0.0f;
  float v1 = 1.0f;

  if (source != nullptr && texture.width() > 0 && texture.height() > 0) {
    const float texWidth = static_cast<float>(texture.width());
    const float texHeight = static_cast<float>(texture.height());
    u0 = source->x / texWidth;
    v0 = source->y / texHeight;
    u1 = (source->x + source->width) / texWidth;
    v1 = (source->y + source->height) / texHeight;
  }

  // Quad corners in local space, relative to the rotation pivot.
  const float pivotX = dest.x + origin.x;
  const float pivotY = dest.y + origin.y;

  math::Vec2 corners[4] = {
      // clang-format off
    {dest.x - pivotX,                dest.y - pivotY},
    {dest.x + dest.width - pivotX,   dest.y - pivotY},
    {dest.x + dest.width - pivotX,   dest.y + dest.height - pivotY},
    {dest.x - pivotX,                dest.y + dest.height - pivotY},
      // clang-format on
  };

  if (rotationRadians != 0.0f) {
    const float c = std::cos(rotationRadians);
    const float s = std::sin(rotationRadians);
    for (math::Vec2 &p : corners) {
      p = {p.x * c - p.y * s, p.x * s + p.y * c};
    }
  }
  for (math::Vec2 &p : corners) {
    p.x += pivotX;
    p.y += pivotY;
  }

  Vertex *v = &this->vertices_[static_cast<std::size_t>(this->quadCount_) * 4];
  v[0] = {corners[0].x, corners[0].y, u0, v0, tint.r, tint.g, tint.b, tint.a};
  v[1] = {corners[1].x, corners[1].y, u1, v0, tint.r, tint.g, tint.b, tint.a};
  v[2] = {corners[2].x, corners[2].y, u1, v1, tint.r, tint.g, tint.b, tint.a};
  v[3] = {corners[3].x, corners[3].y, u0, v1, tint.r, tint.g, tint.b, tint.a};

  ++this->quadCount_;
}

void SpriteBatch::drawRect(const math::Rect &dest, const Color &color) {
  this->draw(this->whiteTexture_, dest, nullptr, color);
}

void SpriteBatch::flush() {
  if (this->quadCount_ == 0)
    return;

  const std::size_t vertexBytes =
      static_cast<std::size_t>(this->quadCount_) * 4 * sizeof(Vertex);

  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->vbo_));
  GL_CHECK(
      glBufferSubData(GL_ARRAY_BUFFER, 0, vertexBytes, this->vertices_.data()));

  GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_));

  GL_CHECK(
      glEnableVertexAttribArray(static_cast<GLuint>(this->attribPosition_)));
  GL_CHECK(glVertexAttribPointer(
      static_cast<GLuint>(this->attribPosition_), 2, GL_FLOAT, GL_FALSE,
      sizeof(Vertex), reinterpret_cast<const void *>(offsetof(Vertex, x))));

  GL_CHECK(
      glEnableVertexAttribArray(static_cast<GLuint>(this->attribTexCoord_)));
  GL_CHECK(glVertexAttribPointer(
      static_cast<GLuint>(this->attribTexCoord_), 2, GL_FLOAT, GL_FALSE,
      sizeof(Vertex), reinterpret_cast<const void *>(offsetof(Vertex, u))));

  GL_CHECK(glEnableVertexAttribArray(static_cast<GLuint>(this->attribColor_)));
  GL_CHECK(glVertexAttribPointer(
      static_cast<GLuint>(this->attribColor_), 4, GL_FLOAT, GL_FALSE,
      sizeof(Vertex), reinterpret_cast<const void *>(offsetof(Vertex, r))));

  GL_CHECK(glBindTexture(GL_TEXTURE_2D, this->currentTexture_));
  GL_CHECK(glDrawElements(GL_TRIANGLES, this->quadCount_ * 6, GL_UNSIGNED_SHORT,
                          nullptr));

  this->quadCount_ = 0;
}

} // namespace salshalon::graphics
