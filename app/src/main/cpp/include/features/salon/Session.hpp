#pragma once

#include <cstdint>
namespace salshalon::features::salon {

struct SessionResult {
  int32_t day = 1;
  int32_t moneyBefore = 0;
  int32_t moneyAfter = 0;
  int32_t reputationBefore = 0;
  int32_t reputationAfter = 0;
  uint32_t customersServed = 0;
  uint32_t customersLost = 0;
  float elapsedSecods = 0.0f;
  uint32_t difficultyScore = 0.0f;

  int32_t moneyEarned() const { return moneyAfter - moneyBefore; }

  int32_t reputationDelta() const { return reputationAfter - reputationBefore; }
};

} // namespace salshalon::features::salon
