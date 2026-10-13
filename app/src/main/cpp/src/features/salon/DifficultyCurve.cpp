#include "features/salon/DifficultyCurve.hpp"

#include <algorithm>
#include <cmath>

namespace salshalon::features::salon::DifficultyCurve {

namespace {
constexpr float kRepSpawnFactorMin = 0.75f;
constexpr float kRepSpawnFactorMax = 1.30f;
} // namespace

float baseSpawnInterval(uint32_t score) {
  const float s = static_cast<float>(score);
  return std::max(2.0f, 10.0f - s * 0.1f);
}

float jitteredSpawnInterval(uint32_t score, int32_t reputation,
                            float randomUnit) {
  const float base = baseSpawnInterval(score);
  const float jitter = 0.8f + randomUnit * 0.4f;
  const float rep = reputationSpawnFactor(reputation);
  return base * jitter * rep;
}

float patienceMultiplier(uint32_t score) {
  const float s = static_cast<float>(score);
  return std::max(0.4f, 1.0f - s * 0.005f);
}

float reputationSpawnFactor(int32_t reputation) {
  const float t = std::clamp(reputation / 100.0f, 0.0f, 1.0f);
  return std::lerp(kRepSpawnFactorMax, kRepSpawnFactorMin, t);
}

} // namespace salshalon::features::salon::DifficultyCurve
