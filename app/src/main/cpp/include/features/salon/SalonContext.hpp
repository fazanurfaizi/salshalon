
#pragma once

#include "features/salon/SalonLayout.hpp"
#include "features/salon/SalonProfile.hpp"
#include <cstdint>

namespace salshalon::features::salon {

class SalonContext {
public:
  SalonContext() = default;

  // Profile
  int32_t money() const { return this->money_; }
  int32_t reputation() const { return this->reputation_; }
  int32_t day() const { return this->day_; }
  uint32_t totalCustomersServed() const { return this->totalCustomersServed_; }
  uint32_t bestDayCustomers() const { return this->bestDayCustomers_; }

  bool earn(int32_t amount);
  bool trySpend(int32_t amount);

  /// Applies a reputation delta, clamped to [0, 100].
  void adjustReputation(int32_t delta);

  void advanceDay();

  // Session events
  void onCustomerServed(int32_t payout);
  void onCustomerLost();

  void recordDayResult(uint32_t served);

  // Layout
  SalonLayout &layout() { return this->layout_; }
  const SalonLayout &layout() const { return this->layout_; }

  // Cosmetics
  const std::string &currentFloorId() const { return this->floorId_; }
  const std::string &currentWallId() const { return this->wallId_; }

  bool isCosmeticUnlocked(const std::string &id) const;
  bool unlockCosmetic(const std::string &id);

  void setCurrentFloor(const std::string &id);
  void setCurrentWall(const std::string &id);

  SalonProfile toProfile() const;

  void applyProfile(const SalonProfile &profile);

private:
  static constexpr int32_t kDailyReputationDecay = 1;
  static constexpr int32_t kReputationMin = 0;
  static constexpr int32_t kReputationMax = 100;

  int32_t money_ = 200;
  int32_t reputation_ = 0;
  int32_t day_ = 1;
  uint32_t totalCustomersServed_ = 0;
  uint32_t bestDayCustomers_ = 0;

  std::string floorId_ = "floor_wood";
  std::string wallId_ = "wall_cream";
  std::vector<std::string> unlockedCosmeticIds_;

  SalonLayout layout_{};
};

} // namespace salshalon::features::salon
