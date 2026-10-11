#pragma once

#include "features/salon/Customer.hpp"
#include "features/salon/SalonAssets.hpp"
#include "features/salon/SalonContext.hpp"
#include "features/salon/SalonGrid.hpp"
#include "game/GameState.hpp"
#include "ui/Button.hpp"
#include "ui/Renderer.hpp"
#include "ui/Toast.hpp"
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

  struct CellCoord {
    int col;
    int row;
  };

  static constexpr CellCoord kDoorCell{0, 5};

  static constexpr std::array<CellCoord, 3> kWaitSlots{{
      {3, 2},
      {4, 2},
      {5, 2},
  }};

  static constexpr std::array<CellCoord, 2> kChairSlots{{
      {8, 1},
      {10, 1},
  }};

  // HUD
  static constexpr float kHudTopHeight = 64.0f;
  static constexpr float kHudBottomHeight = 64.0f;
  static constexpr float kExitButtonWidth = 200.0f;

  static constexpr float kCustomerSize = 64.0f;
  static constexpr float kCustomerHitRadius = 48.0f;

  SalonContext context_{};
  Assets assets_{};
  ui::Renderer renderer_{};

  std::array<Cell, kGridCols * kGridRows> grid_{};

  std::vector<Customer> customers_;

  uint32_t nextCustomerId_{0};

  // @NOTE: delete on next step
  std::unique_ptr<ui::Button> spawnButton_;

  std::unique_ptr<ui::Toast> toast_{};

  // Customer lifecycle
  void spawnCustomer();
  bool seatCustomer(Customer &customer);
  void applyCustomerTransition(Customer &customer);
  void removeGoneCustomers();

  // Reservation helpers
  int findFreeWaitSlot() const;
  int findFreeChair() const;

  void reserveCell(int index, uint32_t customerId);
  void freeCell(int index);
  void freeCustomerCell(Customer &customer);

  // Grid helpers
  Cell &getCell(int col, int row);
  const Cell &getCell(int col, int row) const;

  Cell &cellByIndex(int index);
  const Cell &cellByIndex(int index) const;

  static int cellIndex(const CellCoord &coord);
  math::Vec2 cellCenter(int col, int row) const;
  math::Vec2 cellCenterByIndex(int index) const;

  bool cellFromWorld(const math::Vec2 &p, int &outCol, int &outRow) const;

  graphics::Color cellColor(CellType type, bool alternate) const;

  void initGrid();

  // Rendering
  void renderGrid();
  void renderHud();
  void renderCustomer();
};

} // namespace salshalon::features::salon
