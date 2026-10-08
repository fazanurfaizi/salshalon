#pragma once

#include "graphics/Texture.hpp"

namespace salshalon::core {
class AssetManager;
}

namespace salshalon::features::main_menu {

struct Assets {
  graphics::Texture background;
  graphics::Texture logo;

  bool load(core::AssetManager &assets);

  void unload();
};

} // namespace salshalon::features::main_menu
