#pragma once

#include <cstdint>

namespace salshalon::features::salon::DifficultyCurve {

/// Base patience in seconds, before archetype and difficulty multipliers.
inline constexpr float kBasePatienceSeconds = 25.0f;

/// Base payout in currency, before archetype tip multiplier.
inline constexpr int32_t kBasePayout = 50;

/// Spawn interval without jitter - the deterministic middle of the curve.
/// Used for display and tuning; actual spawns use jitteredSpawnInterval.
float baseSpawnInterval(uint32_t score);

/// Spawn interval with jitter applied. `randomUnit` must be in [0, 1).
/// The jitter varies the interval by +-20%.
float jitteredSpawnInterval(uint32_t score, int32_t reputation,
                            float randomUnit);

/// Multiplier applied to a customer's base patience at the given score.
float patienceMultiplier(uint32_t score);

/// Multiplier on spawn interval derived from reputation.
/// Higher reputation -> smaller multiplier -> faster arrivals.
float reputationSpawnFactor(int32_t reputation);

} // namespace salshalon::features::salon::DifficultyCurve
