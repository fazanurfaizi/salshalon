#include "features/salon/SalonPlayState.hpp"

#include <algorithm>
#include <memory>
#include <string>

#include "core/Engine.hpp"
#include "core/GameConfig.hpp"
#include "core/Log.hpp"
#include "features/Feature.hpp"
#include "features/main_menu/MainMenuState.hpp"
#include "features/salon/Customer.hpp"
#include "graphics/Color.hpp"
#include "graphics/SpriteBatch.hpp"
#include "input/TouchEvent.hpp"
#include "math/Math.hpp"
#include "math/Rect.hpp"
#include "ui/Button.hpp"

namespace salshalon::features::salon {

using core::kDesignHeight;
using core::kDesignWidth;

namespace {
constexpr float kCustomerHitRadius = 48.0f;
}

void SalonPlayState::onEnter(core::Engine &engine) {
  game::GameState::onEnter(engine);
  this->assets_.load(engine.assets());
  this->initGrid();

  // @NOTE: delete on next step
  this->spawnButton_ = std::make_unique<ui::Button>(
      "Spawn", math::Rect{40.0f, 360.0f, 200.0f, 50.0f},
      [this] { this->spawnCustomer(); });
  this->spawnButton_->setTextScale(4.0f);

  SAL_LOGI("Salon: enter");
}

void SalonPlayState::onExit() {
  this->assets_.unload();
  this->customers_.clear();
  this->spawnButton_.reset();
  SAL_LOGI("Salon: exit");
}

void SalonPlayState::update(float deltaSeconds) {
  this->elapsed_ += deltaSeconds;
  this->spawnButton_->update(deltaSeconds);

  for (auto &customer : this->customers_) {
    customer.update(deltaSeconds);
  }

  for (auto &customer : this->customers_) {
    this->applyCustomerTransition(customer);
  }

  this->removeGoneCustomers();
}

void SalonPlayState::applyCustomerTransition(Customer &customer) {
  switch (customer.currentState()) {
  case CustomerState::Arriving:
    if (customer.hasArrived()) {
      customer.setCurrentState(CustomerState::Queueing);
    }
    break;

  case CustomerState::WalkingToSeat:
    if (customer.hasArrived()) {
      customer.setCurrentState(CustomerState::Seated);
    }
    break;

  case CustomerState::Seated:
    if (customer.hasSatLongEnough()) {
      customer.setTargetPosition(cellCenter(kDoorCol, kDoorRow));
      customer.setCurrentState(CustomerState::Leaving);
    }
    break;

  case CustomerState::Queueing:
  case CustomerState::Leaving:
    break;
  }
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

  this->spawnButton_->render(this->renderer_);
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
      const math::Rect cellRect{
          kGridOriginX + col * kTileSize,
          kGridOriginY + row * kTileSize,
          kTileSize,
          kTileSize,
      };

      const bool alternate = ((col + row) & 1) != 0;
      const Cell &cell = getCell(col, row);
      const graphics::Color color = cellColor(cell.type, alternate);
      this->renderer_.drawRect(cellRect.inset(1.0f), color);
    }
  }
}

void SalonPlayState::renderCustomer() {
  // Placeholder customer
  for (const auto &customer : this->customers_) {
    graphics::Color bodyColor;
    switch (customer.currentState()) {
    case CustomerState::Arriving:
    case CustomerState::WalkingToSeat:
      bodyColor = {0.92f, 0.64f, 0.38f, 1.0f}; // warm orange
      break;
    case CustomerState::Queueing:
      bodyColor = {0.92f, 0.82f, 0.38f, 1.0f}; // yellow — waiting
      break;
    case CustomerState::Seated:
      bodyColor = {0.45f, 0.82f, 0.55f, 1.0f}; // green — satisfied
      break;
    case CustomerState::Leaving:
      bodyColor = {0.60f, 0.60f, 0.65f, 1.0f}; // grey — gone
      break;
    }

    const math::Rect rect = math::Rect::fromCenter(
        customer.currentPosition(), this->customerSize_, this->customerSize_);
    this->renderer_.drawRect(rect, bodyColor);

    // Small head so orientation is readable
    const math::Rect head = math::Rect::fromCenter(
        {customer.currentPosition().x,
         customer.currentPosition().y - this->customerSize_ * 0.35f},
        this->customerSize_ * 0.55f, this->customerSize_ * 0.55f);
    this->renderer_.drawRect(head, graphics::Color{1.0f, 0.82f, 0.66f, 1.0f});
  }
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

void SalonPlayState::onTouch(const input::TouchEvent &event) {
  if (this->spawnButton_ && this->spawnButton_->onTouch(event)) {
    return;
  }

  if (event.phase != input::TouchPhase::Down)
    return;

  if (event.phase == input::TouchPhase::Down) {
    // Exit region: top-left conter of the top HUD strip
    if (event.position.y < kHudTopHeight &&
        event.position.x < kExitButtonWidth) {
      SAL_LOGI("Salon: exit tapped");
      goTo<main_menu::MainMenuState>(*this->engine_);
      return;
    }

    Customer *hit = nullptr;
    float bestDist = kCustomerHitRadius;

    for (auto &customer : this->customers_) {
      if (customer.currentState() != CustomerState::Queueing)
        continue;

      const float d = glm::distance(customer.currentPosition(), event.position);
      if (d < bestDist) {
        bestDist = d;
        hit = &customer;
      }
    }
    if (hit != nullptr) {
      this->seatCustomer(*hit);
    }
  }
}

Cell &SalonPlayState::getCell(int col, int row) {
  return this->grid_[row * kGridCols + col];
}

const Cell &SalonPlayState::getCell(int col, int row) const {
  return this->grid_[row * kGridCols + col];
}

void SalonPlayState::initGrid() {
  // Set door at entrance
  getCell(kDoorCol, kDoorRow).type = CellType::Door;

  // Set Waiting Seats
  getCell(kSeatCol, kSeatRow).type = CellType::WaitingSeat;
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

graphics::Color SalonPlayState::cellColor(CellType type, bool alternate) const {
  switch (type) {
  case CellType::Door:
    return graphics::Color{0.45f, 0.25f, 0.15f, 1.0f};
  case CellType::WaitingSeat:
    return graphics::Color{0.20f, 0.40f, 0.55f, 1.0f};
  case CellType::Empty:
  default:
    return alternate ? graphics::Color{0.21f, 0.17f, 0.24f, 1.0f}
                     : graphics::Color{0.18f, 0.15f, 0.21f, 1.0f};
  }
}

void SalonPlayState::spawnCustomer() {
  ++this->nextCustomerId_;

  const math::Vec2 spawnPos = cellCenter(kDoorCol, kDoorRow);
  const math::Vec2 targetPos = cellCenter(kWaitCol, kWaitRow);

  Customer customer(this->nextCustomerId_, CustomerState::Arriving, spawnPos);
  customer.setTargetPosition(targetPos);

  this->customers_.push_back(customer);

  SAL_LOGI("Salon: spawning customer %u", this->nextCustomerId_);
}

void SalonPlayState::seatCustomer(Customer &customer) {
  customer.setTargetPosition(cellCenter(kSeatCol, kSeatRow));
  customer.setCurrentState(CustomerState::WalkingToSeat);
}

void SalonPlayState::removeGoneCustomers() {
  // clang-format off
  this->customers_.erase(
    std::remove_if(
      this->customers_.begin(),
      this->customers_.end(),
      [](const Customer &customer) {
        return customer.currentState() == 
          CustomerState::Leaving && customer.hasArrived();
    }),
    this->customers_.end()
  );
  // clang-format on
}

} // namespace salshalon::features::salon
