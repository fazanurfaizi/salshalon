#include "features/salon/SalonState.hpp"

#include <cmath>

#include "core/Engine.hpp"
#include "core/GameConfig.hpp"
#include "core/Log.hpp"
#include "features/Feature.hpp"
#include "features/main_menu/MainMenuState.hpp"
#include "graphics/SpriteBatch.hpp"
#include "input/TouchEvent.hpp"

namespace salshalon::features::salon {

void SalonState::onEnter(core::Engine &engine) {
  game::GameState::onEnter(engine);
  this->assets_.load(engine.assets());
  SAL_LOGI("Salon: enter");
}

void SalonState::onExit() {
  this->assets_.unload();
  SAL_LOGI("Salon: exit");
}

void SalonState::update(float deltaSeconds) { this->elapsed_ += deltaSeconds; }

void SalonState::render(graphics::SpriteBatch &batch) {
  this->renderer_.setBatch(&batch);

  // Flat salon floor
  this->renderer_.drawRect(
      {0.0f, 0.0f, core::kDesignWidth, core::kDesignHeight},
      graphics::Color{0.18f, 0.13f, 0.10f, 1.0f});

  const float pulse = 0.5f + 0.5f * std::sin(this->elapsed_ * 2.0f);
  this->renderer_.drawTextCentered(
      "SALON", {core::kDesignWidth * 0.5f, core::kDesignHeight * 0.5f}, 6.0f,
      graphics::Color{1.0f, 0.85f, 0.55f, 0.4f + 0.6f * pulse});
  this->renderer_.drawTextCentered(
      "tap to return",
      {core::kDesignWidth * 0.5f, core::kDesignHeight - 120.0f}, 2.5f,
      graphics::Color{0.85f, 0.85f, 0.85f, 0.8f});
}

void SalonState::onTouch(const input::TouchEvent &event) {
  if (event.phase == input::TouchPhase::Up) {
    goTo<main_menu::MainMenuState>(*this->engine_);
  }
}

} // namespace salshalon::features::salon
