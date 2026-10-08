#include "graphics/Shader.hpp"

#include "core/AssetManager.hpp"
#include "core/Log.hpp"
#include "graphics/GLCheck.hpp"
#include <GLES2/gl2.h>

namespace salshalon::graphics {

namespace {
GLuint compileStage(GLenum type, const char *source) {
  const GLuint shader = glCreateShader(type);
  if (shader == 0)
    return 0;

  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled != GL_TRUE) {
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log(length > 1 ? static_cast<size_t>(length) : 1, '\0');
    glGetShaderInfoLog(shader, length, nullptr, log.data());
    SAL_LOGE("Shader compile failed (%s): %s",
             type == GL_VERTEX_SHADER ? "vertex" : "fragment", log.data());
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}
} // namespace

Shader::~Shader() { this->destroy(); }

bool Shader::buildFromSource(const char *vertexSource,
                             const char *fragmentSource) {
  this->destroy();

  const GLuint vertex = compileStage(GL_VERTEX_SHADER, vertexSource);
  if (vertex == 0)
    return false;

  const GLuint fragment = compileStage(GL_FRAGMENT_SHADER, fragmentSource);
  if (fragment == 0) {
    glDeleteShader(vertex);
    return false;
  }

  const GLuint program = glCreateProgram();
  GL_CHECK(glAttachShader(program, vertex));
  GL_CHECK(glAttachShader(program, fragment));
  GL_CHECK(glLinkProgram(program));

  GL_CHECK(glDeleteShader(vertex));
  GL_CHECK(glDeleteShader(fragment));

  GLint linked = GL_FALSE;
  GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &linked));
  if (linked != GL_TRUE) {
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log(length ? static_cast<size_t>(length) : 1, '\0');
    glGetProgramInfoLog(program, length, nullptr, log.data());
    SAL_LOGE("Shadedr link failed: %s", log.data());
    glDeleteProgram(program);
    return false;
  }

  this->program_ = program;
  this->uniforms_.clear();
  return true;
}

bool Shader::buildFromAssets(const core::AssetManager &assets,
                             const char *vertexPath, const char *fragmentPath) {
  const std::string vertexSource = assets.readText(vertexPath);
  const std::string fragmentSource = assets.readText(fragmentPath);

  if (vertexSource.empty() || fragmentSource.empty()) {
    SAL_LOGE("Shader: could not read %s / %s", vertexPath, fragmentPath);
    return false;
  }
  return buildFromSource(vertexSource.c_str(), fragmentSource.c_str());
}

void Shader::bind() const {
  if (this->program_ != 0)
    glUseProgram(this->program_);
}

GLint Shader::uniformLocation(const char *name) {
  const auto it = this->uniforms_.find(name);
  if (it != this->uniforms_.end())
    return it->second;

  const GLint location = glGetUniformLocation(this->program_, name);
  this->uniforms_.emplace(name, location);
  return location;
}

void Shader::setMat4(const char *name, const math::Mat4 &value) {
  glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec4(const char *name, const math::Vec4 &value) {
  glUniform4fv(uniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const char *name, const math::Vec3 &value) {
  glUniform3fv(uniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec2(const char *name, const math::Vec2 &value) {
  glUniform2fv(uniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setFloat(const char *name, float value) {
  glUniform1f(uniformLocation(name), value);
}

void Shader::setInt(const char *name, int value) {
  glUniform1i(uniformLocation(name), value);
}

void Shader::destroy() {
  if (this->program_ != 0) {
    glDeleteProgram(this->program_);
    this->program_ = 0;
  }
  this->uniforms_.clear();
}

} // namespace salshalon::graphics
