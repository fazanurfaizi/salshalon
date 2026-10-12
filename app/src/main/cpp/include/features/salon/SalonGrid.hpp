#pragma once

#include <cstdint>

namespace salshalon::features::salon {

enum class CellType : uint32_t {
  Empty,
  Door,
  WaitSlot,
  Station,
};

struct Cell {
  CellType type{CellType::Empty};
  uint32_t occupantId = 0;
};

} // namespace salshalon::features::salon
