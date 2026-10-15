#pragma once

#include "features/salon/CosmeticCatalog.hpp"
#include "features/salon/Customer.hpp"
#include "features/salon/SalonAssets.hpp"
#include "features/salon/SalonContext.hpp"
#include "features/salon/SalonGrid.hpp"
#include "game/GameState.hpp"
#include "ui/Button.hpp"
#include "ui/Renderer.hpp"
#include "ui/Toast.hpp"
#include <memory>
#include <random>
#include <vector>

namespace salshalon::features::salon {

/// Main gameplay screen
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
  static constexpr float kCustomerHitRadius = 48.0f;
  /// Small offset between the customer centre and the patience bar.
  static constexpr float kPatienceBarHeight = 8.0f;
  static constexpr float kPatienceBarGap = 12.0f;

  // Session
  static constexpr float kFirstSpawnDelaySeconds = 2.0f;

  SalonContext *context_ = nullptr;

  Assets assets_{};
  ui::Renderer renderer_{};
  float elapsed_ = 0.0f;

  std::vector<Customer> customers_;

  uint32_t nextCustomerId_{0};

  float timeUntilNextSpawn_ = kFirstSpawnDelaySeconds;

  uint32_t sessionServed_ = 0;
  uint32_t sessionLost_ = 0;
  int32_t sessionMoneyBefore_ = 0;
  int32_t sessionReputationBefore_ = 0;

  std::mt19937 rng_{0x5A15A10Fu};

#ifndef NDEBUG
  float debugLogTimer_ = 0.0f;
#endif // !NDEBUG

  std::unique_ptr<ui::Button> closeButton_{};
  std::unique_ptr<ui::Toast> toast_{};

  // Customer lifecycle
  void spawnTick(float deltaSeconds);
  void spawnCustomer();
  bool seatCustomer(Customer &customer);
  void applyCustomerTransition(Customer &customer);
  void removeGoneCustomers();

  void onClosePressed();

  // Reservation helpers
  int findFreeWaitSlot() const;
  int findFreeStation() const;

  void reserveCell(int index, uint32_t customerId);
  void freeCell(int index);
  void freeCustomerCell(Customer &customer);

  // Rendering
  void renderGrid();
  void renderDecor();
  void renderStations();
  void renderHud();
  void renderCustomer();
  void renderPatienceBar(const Customer &customer);
#ifndef NDEBUG
  void renderDebugOverlay();
#endif

  graphics::Color cellColor(CellType type, bool alternate) const;
  const graphics::Texture *cellTexture(CellType type) const;
};

} // namespace salshalon::features::salon
