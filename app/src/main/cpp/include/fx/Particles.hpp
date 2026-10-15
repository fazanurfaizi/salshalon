#pragma once

#include <cstddef>
#include <cstdint>

#include "graphics/Color.hpp"
#include "math/Math.hpp"

namespace salshalon::graphics {
class SpriteBatch;
}

namespace salshalon::fx {

enum class ParticleKind {
  Coin,  // gold burst on payment
  Heart, // pink float on happy (served) customer
  Dust,  // grey0tan puff on station use
};

class Particles {
public:
  static constexpr std::size_t kMaxParticles = 256;

  void spawn(math::Vec2 position, ParticleKind kind, int count = 12);
  void update(float deltaSeconds);
  void render(graphics::SpriteBatch &batch) const;
  void clear();

private:
  struct Particle {
    bool alive = false;
    math::Vec2 position{0.0f, 0.0f};
    math::Vec2 velocity{0.0f, 0.0f};
    float life = 0.0f;
    float maxLife = 1.0f;
    float size = 8.0f;
    float gravity = 0.0f;
    float drag = 0.0f;
    graphics::Color color{1.0f, 1.0f, 1.0f, 1.0f};
  };

  float randUnit(); // [0, 1)
  void emitOne(const math::Vec2 &position, ParticleKind kind);

  Particle pool_[kMaxParticles]{};
  std::size_t next_ = 0;
  uint32_t rngState_ = 0x9E3779B9u;
};

} // namespace salshalon::fx
