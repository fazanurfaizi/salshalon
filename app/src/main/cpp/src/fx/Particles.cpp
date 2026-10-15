#include "fx/Particles.hpp"

#include <cmath>

#include "graphics/SpriteBatch.hpp"
#include "math/Rect.hpp"

namespace salshalon::fx {

float Particles::randUnit() {
  this->rngState_ ^= this->rngState_ << 13;
  this->rngState_ ^= this->rngState_ >> 17;
  this->rngState_ ^= this->rngState_ << 5;
  return static_cast<float>(this->rngState_) / static_cast<float>(UINT32_MAX);
}

void Particles::spawn(math::Vec2 position, ParticleKind kind, int count) {
  for (int i = 0; i < count; ++i) {
    this->emitOne(position, kind);
  }
}

void Particles::emitOne(const math::Vec2 &position, ParticleKind kind) {
  Particle &p = this->pool_[next_];
  this->next_ = (this->next_ + 1) % kMaxParticles;

  constexpr float kTau = 6.2831853f;
  p.alive = true;
  p.position = position;

  switch (kind) {
  case ParticleKind::Coin: {
    const float angle = -kTau * 0.25f + (this->randUnit() - 0.5f) * 1.8f;
    const float speed = 140.0f + this->randUnit() * 200.0f;
    p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};
    p.maxLife = p.life = 0.6f + this->randUnit() * 0.3f;
    p.size = 8.0f + this->randUnit() * 5.0f;
    p.gravity = 900.0f;
    p.drag = 0.4f;
    const float warmth = 0.75f + this->randUnit() * 0.2f;
    p.color = {1.0f, warmth, 0.20f, 1.0f};
    break;
  }
  case ParticleKind::Heart: {
    p.velocity = {(this->randUnit() - 0.5f) * 80.0f,
                  -(40.0f + this->randUnit() * 70.0f)};
    p.maxLife = p.life = 0.8f + this->randUnit() * 0.4f;
    p.size = 9.0f + this->randUnit() * 5.0f;
    p.gravity = -60.0f;
    p.drag = 0.6f;
    p.color = {1.0f, 0.42f + this->randUnit() * 0.12f, 0.60f, 1.0f};
    break;
  }
  case ParticleKind::Dust: {
    const float angle = this->randUnit() * kTau;
    const float speed = 50.0f + this->randUnit() * 110.0f;
    p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed * 0.7f};
    p.maxLife = p.life = 0.4f + this->randUnit() * 0.25f;
    p.size = 10.0f + this->randUnit() * 7.0f;
    p.gravity = -30.0f;
    p.drag = 3.0f;
    const float shade = 0.62f + this->randUnit() * 0.15f;
    p.color = {shade, shade * 0.92f, shade * 0.78f, 0.8f};
    break;
  }
  }
}

void Particles::update(float deltaSeconds) {
  if (deltaSeconds <= 0.0f)
    return;
  for (auto &p : this->pool_) {
    if (!p.alive)
      continue;
    p.life -= deltaSeconds;
    if (p.life <= 0.0f) {
      p.alive = false;
      continue;
    }
    p.velocity.y += p.gravity * deltaSeconds;
    if (p.drag > 0.0f) {
      const float keep = 1.0f - p.drag * deltaSeconds;
      p.velocity *= (keep > 0.0f ? keep : 0.0f);
    }
    p.position += p.velocity * deltaSeconds;
  }
}

void Particles::render(graphics::SpriteBatch &batch) const {
  for (const auto &p : this->pool_) {
    if (!p.alive)
      continue;
    const float t = p.life / p.maxLife; // 1 -> 0
    const float alpha = p.color.a * (t < 0.5f ? t * 2.0f : 1.0f);
    const float size = p.size * (0.4f + 0.6f * t);
    batch.drawRect(math::Rect::fromCenter(p.position, size, size),
                   {p.color.r, p.color.g, p.color.b, alpha});
  }
}

void Particles::clear() {
  for (auto &p : pool_) {
    p.alive = false;
  }
  this->next_ = 0;
}

} // namespace salshalon::fx
