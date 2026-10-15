#include "features/salon/cosmetics/CosmeticCatalog.hpp"

namespace salshalon::features::salon {

const std::vector<CosmeticArchetype> &CosmeticCatalog::all() {
  using UC = UnlockCondition;

  static const std::vector<CosmeticArchetype> kCatalog = {
      // Floors
      {
          "floor_wood",
          "Wood",
          CosmeticKind::Floor,
          /*primary*/ {0.28f, 0.20f, 0.14f, 1.0f},
          /*secondary*/ {0.32f, 0.23f, 0.16f, 1.0f},
          /*unlock*/ {UC::Type::Always, 0},
      },
      {
          "floor_tile",
          "Tile",
          CosmeticKind::Floor,
          /*primary*/ {0.20f, 0.22f, 0.28f, 1.0f},
          /*secondary*/ {0.24f, 0.27f, 0.34f, 1.0f},
          /*unlock*/ {UC::Type::TotalServed, 20},
      },
      {
          "floor_marble",
          "Marble",
          CosmeticKind::Floor,
          /*primary*/ {0.40f, 0.38f, 0.44f, 1.0f},
          /*secondary*/ {0.48f, 0.46f, 0.52f, 1.0f},
          /*unlock*/ {UC::Type::Reputation, 80},
      },

      // Walls
      {
          "wall_cream",
          "Cream",
          CosmeticKind::Wall,
          /*primary*/ {0.30f, 0.26f, 0.22f, 1.0f},
          /*secondary*/ {0.0f, 0.0f, 0.0f, 0.0f},
          /*unlock*/ {UC::Type::Always, 0},
      },
      {
          "wall_blue",
          "Blue",
          CosmeticKind::Wall,
          /*primary*/ {0.20f, 0.24f, 0.34f, 1.0f},
          /*secondary*/ {0.0f, 0.0f, 0.0f, 0.0f},
          /*unlock*/ {UC::Type::TotalServed, 40},
      },
      {
          "wall_charcoal",
          "Charcoal",
          CosmeticKind::Wall,
          /*primary*/ {0.14f, 0.14f, 0.16f, 1.0f},
          /*secondary*/ {0.0f, 0.0f, 0.0f, 0.0f},
          /*unlock*/ {UC::Type::BestDay, 15},
      },

      // Decor
      {
          "decor_plant",
          "Plant",
          CosmeticKind::Decor,
          /*primary*/ {0.32f, 0.60f, 0.34f, 1.0f},
          /*secondary*/ {0.0f, 0.0f, 0.0f, 0.0f},
          /*unlock*/ {UC::Type::Always, 0},
      },
      {
          "decor_lamp",
          "Lamp",
          CosmeticKind::Decor,
          /*primary*/ {0.90f, 0.82f, 0.42f, 1.0f},
          /*secondary*/ {0.0f, 0.0f, 0.0f, 0.0f},
          /*unlock*/ {UC::Type::TotalServed, 10},
      },
      {
          "decor_painting",
          "Painting",
          CosmeticKind::Decor,
          /*primary*/ {0.72f, 0.30f, 0.34f, 1.0f},
          /*secondary*/ {0.0f, 0.0f, 0.0f, 0.0f},
          /*unlock*/ {UC::Type::Reputation, 60},
      },
  };
  return kCatalog;
}

const CosmeticArchetype *CosmeticCatalog::find(const std::string &id) {
  for (const auto &c : all()) {
    if (c.id == id)
      return &c;
  }
  return nullptr;
}

std::vector<const CosmeticArchetype *>
CosmeticCatalog::byKind(CosmeticKind kind) {
  std::vector<const CosmeticArchetype *> result;
  for (const auto &c : all()) {
    if (c.kind == kind)
      result.push_back(&c);
  }
  return result;
}

} // namespace salshalon::features::salon
