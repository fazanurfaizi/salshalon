#include "features/salon/SalonLayout.hpp"
#include "features/salon/stations/Station.hpp"

namespace salshalon::features::salon {

SalonLayout::SalonLayout() { this->reset(); }

bool SalonLayout::cellFromWorld(const math::Vec2 &p, int &outCol, int &outRow) {
  if (p.x < kOriginX || p.y < kOriginY)
    return false;

  const int col = static_cast<int>((p.x - kOriginX) / kTileSize);
  const int row = static_cast<int>((p.y - kOriginY) / kTileSize);

  if (col < 0 || col >= kCols)
    return false;
  if (row < 0 || row >= kRows)
    return false;

  outCol = col;
  outRow = row;
  return true;
}

// Stations
const StationInstance *SalonLayout::stationAt(int col, int row) const {
  for (const auto &s : this->stations_) {
    if (s.gridX == col && s.gridY == row)
      return &s;
  }
  return nullptr;
}

bool SalonLayout::canPlaceStation(int col, int row) const {
  if (!this->inBounds(col, row))
    return false;

  const Cell &c = this->cellAt(col, row);
  if (c.type == CellType::Door)
    return false;
  if (c.type == CellType::WaitSlot)
    return false;
  if (c.type == CellType::Station)
    return false;
  return true;
}

bool SalonLayout::placeStation(const StationInstance &inst) {
  if (!this->canPlaceStation(inst.gridX, inst.gridY))
    return false;
  this->stations_.push_back(inst);
  this->rebuildCells();
  return true;
}

std::optional<StationInstance> SalonLayout::removeStationAt(int col, int row) {
  for (auto it = this->stations_.begin(); it != this->stations_.end(); ++it) {
    if (it->gridX == col && it->gridY == row) {
      StationInstance removed = *it;
      this->stations_.erase(it);
      this->rebuildCells();
      return removed;
    }
  }
  return std::nullopt;
}

// Decors
const DecorInstance *SalonLayout::decorAt(int col, int row) const {
  for (const auto &d : this->decors_) {
    if (d.gridX == col && d.gridY == row)
      return &d;
  }
  return nullptr;
}

bool SalonLayout::canPlaceDecor(int col, int row) const {
  if (!inBounds(col, row))
    return false;
  const Cell &c = cellAt(col, row);
  if (c.type == CellType::Door)
    return false;
  if (c.type == CellType::WaitSlot)
    return false;
  if (c.type == CellType::Station)
    return false;
  if (decorAt(col, row) != nullptr)
    return false;
  return true;
}

bool SalonLayout::placeDecor(const DecorInstance &inst) {
  if (!this->canPlaceDecor(inst.gridX, inst.gridY))
    return false;
  this->decors_.push_back(inst);
  return true;
}

std::optional<DecorInstance> SalonLayout::removeDecorAt(int col, int row) {
  for (auto it = this->decors_.begin(); it != this->decors_.end(); ++it) {
    if (it->gridX == col && it->gridY == row) {
      DecorInstance removed = *it;
      this->decors_.erase(it);
      return removed;
    }
  }
  return std::nullopt;
}

void SalonLayout::resetOccupantId() {
  for (auto &c : this->grid_) {
    c.occupantId = 0;
  }
}

void SalonLayout::reset() {
  for (auto &c : this->grid_) {
    c.occupantId = 0;
    c.type = CellType::Empty;
  }
  this->stations_.clear();
  this->decors_.clear();

  StationInstance starter;
  starter.archetypeId = "cutting_chair";
  starter.gridX = 8;
  starter.gridY = 1;
  this->stations_.push_back(starter);

  this->rebuildCells();
}

void SalonLayout::rebuildCells() {
  // Recompute derived cell types. occupantId is preserved
  // it's runtime state owned by the play statem not by the layout
  for (auto &c : this->grid_) {
    c.type = CellType::Empty;
  }

  cellAt(kDoorCell.col, kDoorCell.row).type = CellType::Door;
  for (const auto &w : kWaitSlots) {
    cellAt(w.col, w.row).type = CellType::WaitSlot;
  }
  for (const auto &s : this->stations_) {
    if (inBounds(s.gridX, s.gridY)) {
      cellAt(s.gridX, s.gridY).type = CellType::Station;
    }
  }
}

} // namespace salshalon::features::salon
