#include "features/main_menu/MainMenuAssets.hpp"

#include "core/AssetManager.hpp"
#include "core/Log.hpp"

namespace salshalon::features::main_menu {

bool Assets::load(core::AssetManager &assetManager) {
  if (!background.loadFromAsset(assetManager, "textures/main_menu_bg.png")) {
    background.createGradient(4, 256,
                              graphics::Color{0.06f, 0.07f, 0.13f, 1.0f},
                              graphics::Color{0.24f, 0.12f, 0.26f, 1.0f});
    SAL_LOGI("MainMenuState: using generated background");
  }

  if (!logo.loadFromAsset(assetManager, "textures/logo.png")) {
    SAL_LOGI("MainMenuState: using text title");
  }

  return background.valid();
}

void Assets::unload() {
  background.release();
  logo.release();
}

} // namespace salshalon::features::main_menu
