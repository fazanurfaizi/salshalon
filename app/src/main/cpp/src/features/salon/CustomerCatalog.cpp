#include "features/salon/CustomerCatalog.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <utility>

#include "core/Log.hpp"

namespace salshalon::features::salon {

const std::vector<CustomerArchetype> &CustomerCatalog::all() {
  static const std::vector<CustomerArchetype> kCatalog = {
      {
          /*id*/ "casual",
          /*displayName*/ "Casual",
          /*placeholderColor*/ {0.92f, 0.64f, 0.38f, 1.0f}, // warm orange
          /*patienceMultiplier*/ 1.00f,
          /*tipMultiplier*/ 1.00f,
          /*weightAtLowRep*/ 1.00f,
          /*weightAtHighRep*/ 0.50f,
          /*minReputation*/ 0,
      },
      {
          "business",
          "Business",
          {0.45f, 0.55f, 0.75f, 1.0f}, // blue-grey
          0.70f,
          1.50f,
          0.20f,
          1.00f,
          30,
      },
      {
          "elderly",
          "Elderly",
          {0.55f, 0.75f, 0.55f, 1.0f}, // soft green
          1.50f,
          0.80f,
          1.00f,
          0.30f,
          0,
      },
      {
          "vip",
          "VIP",
          {0.95f, 0.85f, 0.30f, 1.0f}, // gold
          0.60f,
          2.50f,
          0.00f,
          1.00f,
          70,
      },
      {
          "kid",
          "Kid",
          {0.90f, 0.55f, 0.70f, 1.0f}, // pink
          0.50f,
          0.50f,
          0.80f,
          0.20f,
          0,
      },
  };

  return kCatalog;
}

const CustomerArchetype *CustomerCatalog::find(const std::string id) {
  for (const auto &a : all()) {
    if (a.id == id)
      return &a;
  }
  return nullptr;
}

const CustomerArchetype *CustomerCatalog::pickForReputation(int32_t reputation,
                                                            std::mt19937 &rng) {
  const float t = std::clamp(reputation / 100.0f, 0.0f, 1.0f);

  // Build elligible candidate list with interpolated weights.
  std::vector<std::pair<const CustomerArchetype *, float>> candidates;
  candidates.reserve(all().size());
  float total = 0.0f;

  for (const auto &a : all()) {
    if (reputation < a.minReputation)
      continue;
    const float w = std::lerp(a.weightAtLowRep, a.weightAtHighRep, t);
    if (w <= 0.0f)
      continue;
    candidates.emplace_back(&a, w);
    total += w;
  }

  if (candidates.empty() || total <= 0.0f) {
    SAL_LOGW("CustomerCatalog: no eligible archetype at rep %d", reputation);
    return nullptr;
  }

  std::uniform_real_distribution<float> dist(0.0f, total);
  float roll = dist(rng);
  for (const auto &[arch, weight] : candidates) {
    roll -= weight;
    if (roll <= 0.0f)
      return arch;
  }
  return candidates.back().first;
}

} // namespace salshalon::features::salon
