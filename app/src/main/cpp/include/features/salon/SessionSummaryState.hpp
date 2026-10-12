#pragma once

#include "core/Engine.hpp"
#include "features/salon/Session.hpp"
#include "game/GameState.hpp"
#include "graphics/SpriteBatch.hpp"
#include "input/TouchEvent.hpp"
#include "ui/Button.hpp"
#include "ui/Renderer.hpp"
#include <memory>

namespace salshalon::features::salon {

class SessionSummaryState final : public game::GameState {
public:
  explicit SessionSummaryState(SessionResult result);

  void onEnter(core::Engine &engine) override;
  void onExit() override;

  void update(float deltaSeconds) override;
  void render(graphics::SpriteBatch &batch) override;
  void onTouch(const input::TouchEvent &event) override;

private:
  SessionResult result_{};
  ui::Renderer renderer_{};
  std::unique_ptr<ui::Button> continueButton_{};

  void buildUi();
  void onContinuePressed();
};

} // namespace salshalon::features::salon
