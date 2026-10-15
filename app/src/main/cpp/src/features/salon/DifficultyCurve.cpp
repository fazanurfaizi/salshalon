#include "features/salon/DifficultyCurve.hpp"

#include <algorithm>
#include <cmath>

namespace salshalon::features::salon::DifficultyCurve {

namespace {
constexpr float kRepSpawnFactorMin = 0.75f;
constexpr float kRepSpawnFactorMax = 1.30f;
} // namespace

float baseSpawnInterval(uint32_t score) {
  // Tuning (milestone 08): gentler ramp than the original 10 - 0.1*score
  // with a 2s floor, which out-spawned the chairs by session ~6. The 3.5s
  // floor keeps late game busy but clearable; early sessions breathe.
  const float s = static_cast<float>(score);
  return std::max(3.5f, 10.0f - s * 0.08f);
}

float jitteredSpawnInterval(uint32_t score, int32_t reputation,
                            float randomUnit) {
  const float base = baseSpawnInterval(score);
  const float jitter = 0.8f + randomUnit * 0.4f;
  const float rep = reputationSpawnFactor(reputation);
  return base * jitter * rep;
}

float patienceMultiplier(uint32_t score) {
  // Tuning (milestone 08): slower decay (0.004 vs 0.005) and a higher
  // floor (0.55 vs 0.4) so patience never drops below ~14s. Combined with
  // the gentler spawn ramp, sessions 1-10 stay fair; pressure still
  // builds because volume, not the timer, is the constraint.
  const float s = static_cast<float>(score);
  return std::max(0.55f, 1.0f - s * 0.004f);
}

float reputationSpawnFactor(int32_t reputation) {
  const float t = std::clamp(reputation / 100.0f, 0.0f, 1.0f);
  return std::lerp(kRepSpawnFactorMax, kRepSpawnFactorMin, t);
}

} // namespace salshalon::features::salon::DifficultyCurve
