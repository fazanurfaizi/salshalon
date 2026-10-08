#pragma once

#include "game/GameState.hpp"
#include "ui/Button.hpp"
#include "ui/Renderer.hpp"
#include "ui/Toast.hpp"

namespace salshalon::features::settings {

class SettingsState final : public game::GameState {
public:
  void onEnter(core::Engine &engine) override;
  void onExit() override;

  void update(float deltaSeconds) override;
  void render(graphics::SpriteBatch &batch) override;
  void onTouch(const input::TouchEvent &event) override;

private:
  void buildUi();
  void onShowPressed();

  ui::Renderer renderer_{};

  ui::Toast toast_{};
  std::vector<std::unique_ptr<ui::Button>> buttons_{};
};

} // namespace salshalon::features::settings
