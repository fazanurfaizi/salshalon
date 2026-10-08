#pragma once

#include <memory>
#include <string>
#include <vector>

#include "features/main_menu/MainMenuAssets.hpp"
#include "game/GameState.hpp"
#include "ui/Button.hpp"
#include "ui/Renderer.hpp"

namespace salshalon::features::main_menu {

/// The entry screen
class MainMenuState final : public game::GameState {
public:
  MainMenuState() = default;

  void onEnter(core::Engine &engine) override;
  void onExit() override;

  void update(float deltaSeconds) override;
  void render(graphics::SpriteBatch &batch) override;
  void onTouch(const input::TouchEvent &event) override;

private:
  void buildUi();

  void onPlayPressed();
  void onSettingsPressed();
  void onQuitPressed();

  Assets assets_{};
  ui::Renderer renderer_{};
  std::vector<std::unique_ptr<ui::Button>> buttons_{};

  float elapsed_ = 0.0f;
};

} // namespace salshalon::features::main_menu
