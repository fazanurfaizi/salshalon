#pragma once

#include "features/salon/Customer.hpp"
#include "features/salon/SalonAssets.hpp"
#include "features/salon/SalonContext.hpp"
#include "features/salon/SalonGrid.hpp"
#include "game/GameState.hpp"
#include "ui/Button.hpp"
#include "ui/Renderer.hpp"
#include <array>
#include <memory>
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
  // GRID
  static constexpr int kGridCols = 12;
  static constexpr int kGridRows = 6;
  static constexpr float kTileSize = 96.0f;
  static constexpr float kGridOriginX = 64.0f; // centred: (1280 - 12*96)/2
  static constexpr float kGridOriginY = 72.0f; // below the top HUD strip

  // DOOR
  static constexpr int kDoorCol = 0;
  static constexpr int kDoorRow = 5;

  // SEAT
  static constexpr int kSeatCol = 11;
  static constexpr int kSeatRow = 0;

  static constexpr int kWaitCol = 3;
  static constexpr int kWaitRow = 3;

  // HUD
  static constexpr float kHudTopHeight = 64.0f;
  static constexpr float kHudBottomHeight = 64.0f;
  static constexpr float kExitButtonWidth = 200.0f;

  float customerSize_ = 64.0f;

  SalonContext context_{};
  Assets assets_{};
  ui::Renderer renderer_{};
  float elapsed_ = 0.0f;

  std::vector<Customer> customers_;

  uint32_t nextCustomerId_{0};

  // @NOTE: delete on next step
  std::unique_ptr<ui::Button> spawnButton_;
  void spawnCustomer();
  void seatCustomer(Customer &customer);
  void applyCustomerTransition(Customer &customer);
  void removeGoneCustomers();

  std::array<Cell, kGridCols * kGridRows> grid_{};

  Cell &getCell(int col, int row);
  const Cell &getCell(int col, int row) const;

  void initGrid();

  math::Vec2 cellCenter(int col, int row) const;
  bool cellFromWorld(const math::Vec2 &p, int &outCol, int &outRow) const;

  graphics::Color cellColor(CellType type, bool alternate) const;

  void renderGrid();
  void renderHud();
  void renderCustomer();
};

} // namespace salshalon::features::salon
