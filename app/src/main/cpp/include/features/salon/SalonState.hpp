#pragma once

#include "features/salon/SalonAssets.hpp"
#include "game/GameState.hpp"
#include "ui/Renderer.hpp"

namespace salshalon::features::salon {

/// Main gameplay screen
class SalonState final : public game::GameState {
public:
  void onEnter(core::Engine &engine) override;
  void onExit() override;

  void update(float deltaSeconds) override;
  void render(graphics::SpriteBatch &batch) override;
  void onTouch(const input::TouchEvent &event) override;

private:
  Assets assets_{};
  ui::Renderer renderer_{};
  float elapsed_ = 0.0f;
};

} // namespace salshalon::features::salon
