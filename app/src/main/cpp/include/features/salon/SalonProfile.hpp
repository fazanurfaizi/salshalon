#pragma once

#include <cstdint>
#include <vector>

#include "features/salon/Station.hpp"

namespace salshalon::features::salon {

struct SalonProfile {
  static constexpr uint32_t kCurrentVersion = 2;

  uint32_t version = kCurrentVersion;

  int32_t money = 200;
  int32_t reputation = 0;
  uint32_t day = 1;

  uint32_t totalCustomersServed = 0;
  uint32_t bestDayCustomers = 0;

  std::vector<StationInstance> stations;
  std::vector<DecorInstance> decors;

  std::string floorId = "floor_wood";
  std::string wallId = "wall_cream";
  std::vector<std::string> unlockedCosmeticIds;

  bool hasSeenTutorial = false;
};

} // namespace salshalon::features::salon
