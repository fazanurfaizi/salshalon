#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include <graphics/Color.hpp>

namespace salshalon::features::salon {

/// Static description of a kind of customer. Each value is a multiplier or
/// a weight, never an absolute - archetypes compose with the difficulty
/// curve at spawn time.
struct CustomerArchetype {
  std::string id;
  std::string displayName;
  graphics::Color placeholderColor;

  /// Multipliers applied to base values at spawn time.
  float patienceMultiplier = 1.0f;
  float tipMultiplier = 1.0f;

  /// Spawn weighting. Linearly interpolated between low- and high-reputation
  /// weights based on the player's reputation (0..100).
  float weightAtLowRep = 1.0f;
  float weightAtHighRep = 1.0f;

  /// Minimum reputation for this archetype to be eligible.
  int32_t minReputation = 0;
};

class CustomerCatalog {
public:
  static const std::vector<CustomerArchetype> &all();
  static const CustomerArchetype *find(const std::string id);

  /// Weighted-random selection gated by reputation. Returns nullptr only if
  /// no archetype is eligible - impossible in practice, since "casual" has
  /// minReputation == 0.
  static const CustomerArchetype *pickForReputation(int32_t reputation,
                                                    std::mt19937 &rng);
};

} // namespace salshalon::features::salon
