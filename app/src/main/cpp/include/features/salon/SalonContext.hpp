
#pragma once

#include "features/salon/SalonLayout.hpp"
#include <cstdint>

namespace salshalon::features::salon {

class SalonContext {
public:
  SalonContext() = default;

  // Profile
  int32_t money() const { return this->money_; }
  int32_t reputation() const { return this->reputation_; }
  int32_t day() const { return this->day_; }

  bool earn(int32_t amount);
  bool trySpend(int32_t amount);

  /// Applies a reputation delta, clamped to [0, 100].
  void adjustReputation(int32_t delta);

  void advanceDay();

  // Session events
  void onCustomerServed(int32_t payout);
  void onCustomerLost();

  // Layout
  SalonLayout &layout() { return this->layout_; }
  const SalonLayout &layout() const { return this->layout_; }

private:
  static constexpr int32_t kDailyReputationDecay = 1;
  static constexpr int32_t kReputationMin = 0;
  static constexpr int32_t kReputationMax = 100;

  int32_t money_ = 200;
  int32_t reputation_ = 0;
  int32_t day_ = 1;

  SalonLayout layout_{};
};

} // namespace salshalon::features::salon
