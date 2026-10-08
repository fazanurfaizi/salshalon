#pragma once

#include "features/salon/SalonAssets.hpp"
#include "features/salon/SalonContext.hpp"
#include "game/GameState.hpp"
#include "ui/Renderer.hpp"

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

  // HUD
  static constexpr float kHudTopHeight = 64.0f;
  static constexpr float kHudBottomHeight = 64.0f;
  static constexpr float kExitButtonWidth = 200.0f;

  // Placeholder Customer
  math::Vec2 customerPosition_{640.0f, 360.0f};
  float customerSize_ = 64.0f;

  SalonContext context_{};
  Assets assets_{};
  ui::Renderer renderer_{};
  float elapsed_ = 0.0f;

  math::Vec2 cellCenter(int col, int row) const;
  bool cellFromWorld(const math::Vec2 &p, int &outCol, int &outRow) const;

  void renderGrid();
  void renderHud();
  void renderCustomer();
};

} // namespace salshalon::features::salon
