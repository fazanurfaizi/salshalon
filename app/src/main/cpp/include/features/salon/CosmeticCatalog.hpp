#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "graphics/Color.hpp"

namespace salshalon::features::salon {

enum class CosmeticKind {
  Floor,
  Wall,
  Decor,
};

struct UnlockCondition {
  enum class Type { Always, TotalServed, Reputation, BestDay };
  Type type = Type::Always;
  int32_t threshold = 0;
};

struct CosmeticArchetype {
  std::string id;
  std::string displayName;
  CosmeticKind kind;

  graphics::Color primary;
  graphics::Color secondary;

  UnlockCondition unlock;
};

class CosmeticCatalog {
public:
  static const std::vector<CosmeticArchetype> &all();
  static const CosmeticArchetype *find(const std::string &id);
  static std::vector<const CosmeticArchetype *> byKind(CosmeticKind kind);
};

} // namespace salshalon::features::salon
