#pragma once

#include "graphics/Color.hpp"

namespace salshalon::features::salon {
enum class CellType : unsigned int;
}
namespace salshalon::ui {
class Renderer;
}
namespace salshalon::features::salon {
class SalonContext;
}

namespace salshalon::features::salon {

class SalonRenderer {
public:
  static void renderRoom(ui::Renderer &ui, const SalonContext &context,
                         bool showOccupants);

  static graphics::Color cellColor(CellType type, bool alternate);

private:
  static void renderFloor(ui::Renderer &ui, const SalonContext &context,
                          bool showOccupants);
  static void renderStations(ui::Renderer &ui, const SalonContext &context);
  static void renderDecor(ui::Renderer &ui, const SalonContext &context);
};

} // namespace salshalon::features::salon
