#pragma once

#include <cstdint>
#include <vector>

#include "features/salon/Station.hpp"

namespace salshalon::features::salon {

struct SalonProfile {
  static constexpr uint32_t kCurrentVersion = 1;

  uint32_t version = kCurrentVersion;

  int32_t money = 200;
  int32_t reputation = 0;
  uint32_t day = 1;

  uint32_t totalCustomersServed = 0;
  uint32_t bestDayCustomers = 0;

  std::vector<StationInstance> stations;
};

} // namespace salshalon::features::salon
