#pragma once

#include <cstdint>

namespace salshalon::features::salon {

enum class CellType : uint32_t {
  Empty,
  Door,
  WaitingSeat,
};

struct Cell {
  CellType type{CellType::Empty};
  bool occupied{false};
};

} // namespace salshalon::features::salon
