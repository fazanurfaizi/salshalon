#pragma once

#include "graphics/Texture.hpp"

namespace salshalon::core {
class AssetManager;
}

namespace salshalon::features::salon {

struct Assets {
  graphics::Texture floor;
  graphics::Texture door;
  graphics::Texture chair;

  bool load(core::AssetManager &assets);
  void unload();
};

} // namespace salshalon::features::salon
