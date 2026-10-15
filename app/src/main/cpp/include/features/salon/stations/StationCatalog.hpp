#pragma once

#include "features/salon/stations/Station.hpp"
#include <string>
#include <vector>

namespace salshalon::features::salon {

/// Hardcoded registry of station archetypes.
/// Replaced by JSON loading in a later milestone.
class StationCatalog {
public:
  static const std::vector<StationArchetype> &all();
  static const StationArchetype *find(const std::string &id);
};

} // namespace salshalon::features::salon
