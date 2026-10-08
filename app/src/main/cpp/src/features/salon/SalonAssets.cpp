#include "features/salon/SalonAssets.hpp"

#include "core/AssetManager.hpp"

namespace salshalon::features::salon {

bool Assets::load(core::AssetManager &assetManager) {
  floor.loadFromAsset(assetManager, "features/salon/floor.png");
  return true;
}

void Assets::unload() { this->floor.release(); }

} // namespace salshalon::features::salon
