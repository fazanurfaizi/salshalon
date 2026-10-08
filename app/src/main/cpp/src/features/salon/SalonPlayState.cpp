#include "features/salon/SalonPlayState.hpp"

#include <cmath>
#include <string>

#include "core/Engine.hpp"
#include "core/GameConfig.hpp"
#include "core/Log.hpp"
#include "features/Feature.hpp"
#include "features/main_menu/MainMenuState.hpp"
#include "graphics/Color.hpp"
#include "graphics/SpriteBatch.hpp"
#include "input/TouchEvent.hpp"
#include "math/Math.hpp"

namespace salshalon::features::salon {

using core::kDesignHeight;
using core::kDesignWidth;

void SalonPlayState::onEnter(core::Engine &engine) {
  game::GameState::onEnter(engine);
  this->assets_.load(engine.assets());
  SAL_LOGI("Salon: enter");
}

void SalonPlayState::onExit() {
  this->assets_.unload();
  SAL_LOGI("Salon: exit");
}

void SalonPlayState::update(float deltaSeconds) {
  this->elapsed_ += deltaSeconds;
  (void)deltaSeconds;
  // @TODO: customer state machine and movement
}

void SalonPlayState::render(graphics::SpriteBatch &batch) {
  this->renderer_.setBatch(&batch);

  // Flat salon floor
  this->renderer_.drawRect(
      {0.0f, 0.0f, core::kDesignWidth, core::kDesignHeight},
      graphics::Color{0.18f, 0.13f, 0.10f, 1.0f});

  this->renderGrid();
  this->renderCustomer();
  this->renderHud();
}

void SalonPlayState::onTouch(const input::TouchEvent &event) {
  if (event.phase == input::TouchPhase::Down) {
    // Exit region: top-left conter of the top HUD strip
    if (event.position.y < kHudTopHeight &&
        event.position.x < kExitButtonWidth) {
      SAL_LOGE("Salon: exit tapped");
      goTo<main_menu::MainMenuState>(*this->engine_);
      return;
    }

    int col = 0;
    int row = 0;
    if (this->cellFromWorld(event.position, col, row)) {
      SAL_LOGI("salon: tapped cell (%d, %d)", col, row);
      this->customerPosition_ = cellCenter(col, row);
    }
  }
}

void SalonPlayState::renderGrid() {
  // Floor background under the tiles
  const math::Rect floorRect{
      kGridOriginX,
      kGridOriginY,
      kGridCols * kTileSize,
      kGridRows * kTileSize,
  };
  this->renderer_.drawRect(floorRect,
                           graphics::Color{0.16f, 0.13f, 0.18f, 1.0f});

  // Checkerboard tiles
  for (int row = 0; row < kGridRows; ++row) {
    for (int col = 0; col < kGridCols; ++col) {
      const math::Rect cell{
          kGridOriginX + col * kTileSize,
          kGridOriginY + row * kTileSize,
          kTileSize,
          kTileSize,
      };

      const bool alternate = ((col + row) & 1) != 0;
      const graphics::Color c =
          alternate ? graphics::Color{0.21f, 0.17f, 0.24f, 1.0f}
                    : graphics::Color{0.18f, 0.15f, 0.21f, 1.0f};

      this->renderer_.drawRect(cell.inset(1.0f), c);
    }
  }
}

void SalonPlayState::renderCustomer() {
  // Placeholder customer
  const math::Rect rect = math::Rect::fromCenter(
      this->customerPosition_, this->customerSize_, this->customerSize_);
  this->renderer_.drawRect(rect, graphics::Color{0.92f, 0.64f, 0.38f, 1.0f});

  // Small head so orientation is readable
  const math::Rect head = math::Rect::fromCenter(
      {this->customerPosition_.x,
       this->customerPosition_.y - this->customerSize_ * 0.35f},
      this->customerSize_ * 0.55f, this->customerSize_ * 0.55f);
  this->renderer_.drawRect(head, graphics::Color{1.0f, 0.82f, 0.66f, 1.0f});
}

void SalonPlayState::renderHud() {
  // Top
  this->renderer_.drawRect({0.0f, 0.0f, kDesignWidth, kHudTopHeight},
                           graphics::Color{0.04f, 0.04f, 0.07f, 1.0f});

  this->renderer_.drawText("DAY " + std::to_string(this->context_.day()),
                           {24.0f, kHudTopHeight * 0.5f - 12.0f}, 2.5f,
                           graphics::kWhite);

  // Money — right-aligned
  const std::string moneyStr = "$" + std::to_string(this->context_.money());
  const float moneyWidth = this->renderer_.measureText(moneyStr, 2.5f);
  this->renderer_.drawText(
      moneyStr,
      {kDesignWidth - 24.0f - moneyWidth, kHudTopHeight * 0.5f - 12.0f}, 2.5f,
      graphics::Color{0.85f, 1.0f, 0.75f, 1.0f});

  // Bottom
  this->renderer_.drawRect(
      {0.0f, kDesignHeight - kHudBottomHeight, kDesignWidth, kHudBottomHeight},
      graphics::Color{0.04f, 0.04f, 0.07f, 1.0f});

  // Exit hint
  this->renderer_.drawText(
      "TAP TOP-LEFT TO EXIT",
      {24.0f, kDesignHeight - kHudBottomHeight * 0.5f - 12.0f}, 2.2f,
      graphics::Color{0.55f, 0.55f, 0.65f, 1.0f});
}

math::Vec2 SalonPlayState::cellCenter(int col, int row) const {
  return {kGridOriginX + (col + 0.5f) * kTileSize,
          kGridOriginY + (row + 0.5f) * kTileSize};
}

bool SalonPlayState::cellFromWorld(const math::Vec2 &p, int &outCol,
                                   int &outRow) const {
  if (p.x < kGridOriginX || p.y < kGridOriginY)
    return false;

  const int col = static_cast<int>((p.x - kGridOriginX) / kTileSize);
  const int row = static_cast<int>((p.y - kGridOriginY) / kTileSize);

  if (col < 0 || col >= kGridCols)
    return false;
  if (row < 0 || row >= kGridRows)
    return false;

  outCol = col;
  outRow = row;
  return true;
}

} // namespace salshalon::features::salon
