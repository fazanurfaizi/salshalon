#pragma once

#include <GLES2/gl2.h>
#include <string>
#include <unordered_map>

#include "math/Math.hpp"

namespace salshalon::core {
class AssetManager;
}

namespace salshalon::graphics {

// GLSL program warapper with a cached uniform-location table.
class Shader {
public:
  Shader() = default;
  ~Shader();

  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;

  bool buildFromSource(const char *vertexSource, const char *fragmentSource);
  bool buildFromAssets(const core::AssetManager &assets, const char *vertexPath,
                       const char *fragmentPath);

  void bind() const;
  GLuint id() const { return this->program_; }
  bool valid() const { return this->program_ != 0; }

  void destroy();

  void setMat4(const char *name, const math::Mat4 &value);
  void setVec4(const char *name, const math::Vec4 &value);
  void setVec3(const char *name, const math::Vec3 &value);
  void setVec2(const char *name, const math::Vec2 &value);
  void setFloat(const char *name, float value);
  void setInt(const char *name, int value);

private:
  GLint uniformLocation(const char *name);

  GLuint program_ = 0;
  std::unordered_map<std::string, GLint> uniforms_{};
};

} // namespace salshalon::graphics
