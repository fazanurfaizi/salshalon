#pragma once

#include "features/salon/SalonGrid.hpp"
#include "features/salon/Station.hpp"
#include "math/Math.hpp"
#include <array>
#include <vector>

namespace salshalon::features::salon {

class SalonLayout {
public:
  static constexpr int kCols = 12;
  static constexpr int kRows = 6;
  static constexpr float kTileSize = 96.0f;
  static constexpr float kOriginX = 64.0f; // centred: (1280 - 12*96)/2
  static constexpr float kOriginY = 72.0f; // below the top HUD strip

  struct CellCoord {
    int col;
    int row;
  };

  static constexpr CellCoord kDoorCell{0, 5};

  static constexpr std::array<CellCoord, 3> kWaitSlots{{
      {3, 2},
      {4, 2},
      {5, 2},
  }};

  SalonLayout();

  // Grid
  Cell &cellAt(int col, int row) { return this->grid_[cellIndex(col, row)]; }
  const Cell &cellAt(int col, int row) const {
    return this->grid_[cellIndex(col, row)];
  }

  bool inBounds(int col, int row) const {
    return col >= 0 && col < kCols && row >= 0 && row < kRows;
  }

  static int cellIndex(int col, int row) { return row * kCols + col; }

  static math::Vec2 cellCenter(int col, int row) {
    return {kOriginX + (col + 0.5f) * kTileSize,
            kOriginY + (row + 0.5f) * kTileSize};
  }
  static math::Vec2 cellCenterByIndex(int index) {
    return cellCenter(index % kCols, index / kCols);
  }

  static bool cellFromWorld(const math::Vec2 &p, int &outCol, int &outRow);

  // Stations
  const std::vector<StationInstance> &stations() const {
    return this->stations_;
  }
  const StationInstance *stationAt(int col, int row) const;

  /// True if a new station can be placed at (col, row). Checks bounds,
  /// blocked cells (door, wait slots), and existing stations.
  bool canPlaceStation(int col, int row) const;

  /// Adds a station. The cell type is set to Station. Returns false if the
  /// position is invalid — this is a raw placement, no money is charged.
  bool placeStation(const StationInstance &inst);

  /// Removes the station at (col, row) if any. Returns the removed instance
  /// (caller is expected to handle refunds). Cell becomes Empty.
  std::optional<StationInstance> removeStationAt(int col, int row);

  /// Resets the layout to its starting configuration: door, wait slots, and
  /// a single cutting chair so the first session is playable.
  void reset();

private:
  std::array<Cell, kCols * kRows> grid_{};
  std::vector<StationInstance> stations_{};

  void rebuildCells();
};

} // namespace salshalon::features::salon
