#pragma once

#include "graphics/Color.hpp"
#include <string>

namespace salshalon::features::salon {

/// One placed station in the salon. A station is a physical piece of
/// furniture the player has bought.
struct StationInstance {
  std::string archetypeId;
  int gridX = 0;
  int gridY = 0;
};

/// Static description of a station kind. Loaded from StationCatalog.
struct StationArchetype {
  std::string id;
  std::string displayName;
  int32_t cost = 0;
  graphics::Color placeholderColor{1.0f, 1.0f, 1.0f, 1.0f};
};

} // namespace salshalon::features::salon
