#pragma once

#include <cstdint>
#include <memory>

#include "features/salon/SalonAssets.hpp"
#include "features/salon/SalonContext.hpp"
#include "features/salon/play/CustomerManager.hpp"
#include "features/salon/play/Tutorial.hpp"
#include "fx/Particles.hpp"
#include "game/GameState.hpp"
#include "ui/Button.hpp"
#include "ui/Renderer.hpp"
#include "ui/Toast.hpp"

namespace salshalon::features::salon {

/// Main gameplay screen.
class SalonPlayState final : public game::GameState {
public:
  void onEnter(core::Engine &engine) override;
  void onExit() override;

  void update(float deltaSeconds) override;
  void render(graphics::SpriteBatch &batch) override;
  void onTouch(const input::TouchEvent &event) override;

private:
  // HUD
  static constexpr float kHudTopHeight = 64.0f;
  static constexpr float kHudBottomHeight = 64.0f;

  // Customer visuals
  static constexpr float kCustomerSize = 64.0f;
  /// Small offset between the customer centre and the patience bar.
  static constexpr float kPatienceBarHeight = 8.0f;
  static constexpr float kPatienceBarGap = 12.0f;

  SalonContext *context_ = nullptr;

  Assets assets_{};
  ui::Renderer renderer_{};
  CustomerManager manager_{};
  CustomerEvents events_{};
  Tutorial tutorial_{};
  fx::Particles particles_{};

#ifndef NDEBUG
  float debugLogTimer_ = 0.0f;
#endif // !NDEBUG

  int32_t sessionMoneyBefore_ = 0;
  int32_t sessionReputationBefore_ = 0;

  std::unique_ptr<ui::Button> closeButton_{};
  std::unique_ptr<ui::Toast> toast_{};

  void consumeEvents();

  void completeTutorial();

  void onClosePressed();

  void renderCustomer();
  void renderPatienceBar(const Customer &customer);
  void renderHud();
#ifndef NDEBUG
  void renderDebugOverlay();
#endif
};

} // namespace salshalon::features::salon
