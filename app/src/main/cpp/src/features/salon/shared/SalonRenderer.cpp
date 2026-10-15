#include "features/salon/shared/SalonRenderer.hpp"

#include "core/GameConfig.hpp"
#include "features/salon/cosmetics/CosmeticCatalog.hpp"
#include "features/salon/SalonContext.hpp"
#include "features/salon/SalonGrid.hpp"
#include "features/salon/SalonLayout.hpp"
#include "features/salon/stations/StationCatalog.hpp"
#include "ui/Renderer.hpp"

namespace salshalon::features::salon {

void SalonRenderer::renderRoom(ui::Renderer &ui, const SalonContext &context,
                               bool showOccupants) {
  // Wall color as the fullscreen background.
  const auto *wallArch = CosmeticCatalog::find(context.currentWallId());
  const graphics::Color wallColor =
      wallArch ? wallArch->primary : graphics::Color{0.30f, 0.26f, 0.22f, 1.0f};
  ui.drawRect({0.0f, 0.0f, core::kDesignWidth, core::kDesignHeight},
              wallColor);

  renderFloor(ui, context, showOccupants);
  renderStations(ui, context);
  renderDecor(ui, context);
}

graphics::Color SalonRenderer::cellColor(CellType type, bool alternate) {
  switch (type) {
  case CellType::Door:
    return {0.45f, 0.25f, 0.15f, 1.0f};
  case CellType::WaitSlot:
    return {0.20f, 0.40f, 0.55f, 1.0f};
  case CellType::Station:
    return {0.40f, 0.28f, 0.42f, 1.0f};
  case CellType::Empty:
  default:
    return alternate ? graphics::Color{0.21f, 0.17f, 0.24f, 1.0f}
                     : graphics::Color{0.18f, 0.15f, 0.21f, 1.0f};
  }
}

void SalonRenderer::renderFloor(ui::Renderer &ui, const SalonContext &context,
                                bool showOccupants) {
  using L = SalonLayout;

  const auto *floorArch = CosmeticCatalog::find(context.currentFloorId());
  const graphics::Color tileA =
      floorArch ? floorArch->primary
                : graphics::Color{0.28f, 0.20f, 0.14f, 1.0f};
  const graphics::Color tileB =
      floorArch ? floorArch->secondary
                : graphics::Color{0.32f, 0.23f, 0.16f, 1.0f};

  const math::Rect floorRect{
      L::kOriginX,
      L::kOriginY,
      L::kCols * L::kTileSize,
      L::kRows * L::kTileSize,
  };
  ui.drawRect(floorRect, tileA);

  const SalonLayout &layout = context.layout();
  for (int row = 0; row < L::kRows; ++row) {
    for (int col = 0; col < L::kCols; ++col) {
      const math::Rect cellRect{
          L::kOriginX + col * L::kTileSize,
          L::kOriginY + row * L::kTileSize,
          L::kTileSize,
          L::kTileSize,
      };
      const bool alternate = ((col + row) & 1) != 0;
      const Cell &cell = layout.cellAt(col, row);

      graphics::Color fill;
      if (cell.type == CellType::Empty) {
        fill = alternate ? tileB : tileA;
      } else {
        fill = cellColor(cell.type, alternate);
      }
      ui.drawRect(cellRect.inset(1.0f), fill);

      if (showOccupants && cell.occupantId != 0) {
        ui.drawRect(cellRect.inset(L::kTileSize * 0.35f),
                    graphics::Color{1.0f, 1.0f, 1.0f, 0.15f});
      }
    }
  }
}

void SalonRenderer::renderStations(ui::Renderer &ui,
                                   const SalonContext &context) {
  using L = SalonLayout;
  const SalonLayout &layout = context.layout();

  for (const auto &inst : layout.stations()) {
    const math::Rect cellRect{
        L::kOriginX + inst.gridX * L::kTileSize,
        L::kOriginY + inst.gridY * L::kTileSize,
        L::kTileSize,
        L::kTileSize,
    };

    const StationArchetype *arch = StationCatalog::find(inst.archetypeId);
    const graphics::Color color =
        arch ? arch->placeholderColor : graphics::Color{0.7f, 0.7f, 0.7f, 1.0f};

    ui.drawRect(cellRect.inset(8.0f), color);
    ui.drawFrame(cellRect.inset(8.0f),
                 graphics::Color{1.0f, 1.0f, 1.0f, 0.30}, 2.0f);
  }
}

void SalonRenderer::renderDecor(ui::Renderer &ui, const SalonContext &context) {
  using L = SalonLayout;
  for (const auto &d : context.layout().decors()) {
    const math::Rect cellRect{
        L::kOriginX + d.gridX * L::kTileSize,
        L::kOriginY + d.gridY * L::kTileSize,
        L::kTileSize,
        L::kTileSize,
    };

    const auto *arch = CosmeticCatalog::find(d.archetypeId);
    const graphics::Color c =
        arch ? arch->primary : graphics::Color{0.7f, 0.7f, 0.7f, 1.0f};

    ui.drawRect(cellRect.inset(L::kTileSize * 0.20f), c);
    ui.drawFrame(cellRect.inset(L::kTileSize * 0.20f),
                 graphics::Color{1.0f, 1.0f, 1.0f, 0.25f}, 2.0f);
  }
}

} // namespace salshalon::features::salon
