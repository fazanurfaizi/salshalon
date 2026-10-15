#include "features/salon/stations/StationCatalog.hpp"

namespace salshalon::features::salon {

const std::vector<StationArchetype> &StationCatalog::all() {
  static const std::vector<StationArchetype> kStations = {
      {"cutting_chair", "Chair", 200, {0.40f, 0.28f, 0.42f, 1.0f}},
      {"wash_basin", "Basin", 350, {0.30f, 0.45f, 0.60f, 1.0f}},
      {"dryer", "Dryer", 400, {0.60f, 0.45f, 0.30f, 1.0f}},
  };
  return kStations;
}

const StationArchetype *StationCatalog::find(const std::string &id) {
  for (const auto &s : all()) {
    if (s.id == id)
      return &s;
  }
  return nullptr;
}

} // namespace salshalon::features::salon
