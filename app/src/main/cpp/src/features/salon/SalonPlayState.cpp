#include "features/salon/SalonPlayState.hpp"

#include <glm/geometric.hpp>
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

void SalonPlayState::onEnter(core::Engine &engine) {
  game::GameState::onEnter(engine);
  this->assets_.load(engine.assets());
  this->initGrid();

  // @NOTE: delete on next step
  this->spawnButton_ = std::make_unique<ui::Button>(
      "Spawn", math::Rect{40.0f, 360.0f, 200.0f, 50.0f},
      [this] { this->spawnCustomer(); });
  this->spawnButton_->setTextScale(4.0f);

  this->toast_ = std::make_unique<ui::Toast>();
  this->toast_->setKind(ui::ToastKind::Info).setFadeTime(1.5f);

  SAL_LOGI("Salon: enter");
}

void SalonPlayState::onExit() {
  this->assets_.unload();
  this->customers_.clear();
  this->spawnButton_.reset();
  this->toast_.reset();
  SAL_LOGI("Salon: exit");
}

void SalonPlayState::update(float deltaSeconds) {
  this->spawnButton_->update(deltaSeconds);
  this->toast_->update(deltaSeconds);

  for (auto &customer : this->customers_) {
    customer.update(deltaSeconds);
  }

  for (auto &customer : this->customers_) {
    this->applyCustomerTransition(customer);
  }

  this->removeGoneCustomers();
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

  this->toast_->render(this->renderer_,
                       {core::kDesignWidth * 0.5f, core::kDesignHeight - 70.0f},
                       ui::ToastAnchor::Center);
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
      this->renderer_.drawRect(cellRect.inset(1.0f),
                               cellColor(cell.type, alternate));

      if (cell.occupantId != 0) {
        // @NOTE: fade marker on reserved cells. Remove when customer sprites
        this->renderer_.drawRect(cellRect.inset(kTileSize * 0.35f),
                                 graphics::Color{1.0f, 1.0f, 1.0f, 0.15f});
      }
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
        customer.currentPosition(), kCustomerSize, kCustomerSize);
    this->renderer_.drawRect(rect, bodyColor);

    // Small head so orientation is readable
    const math::Rect head = math::Rect::fromCenter(
        {customer.currentPosition().x,
         customer.currentPosition().y - kCustomerSize * 0.35f},
        kCustomerSize * 0.55f, kCustomerSize * 0.55f);
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
  getCell(kDoorCell.col, kDoorCell.row).type = CellType::Door;

  for (const CellCoord &coord : kWaitSlots) {
    getCell(coord.col, coord.row).type = CellType::WaitSlot;
  }
  for (const CellCoord &coord : kChairSlots) {
    getCell(coord.col, coord.row).type = CellType::Chair;
  }
}

Cell &SalonPlayState::cellByIndex(int index) { return this->grid_[index]; }

const Cell &SalonPlayState::cellByIndex(int index) const {
  return this->grid_[index];
}

int SalonPlayState::cellIndex(const CellCoord &coord) {
  return coord.row * kGridCols + coord.col;
}

math::Vec2 SalonPlayState::cellCenter(int col, int row) const {
  return {kGridOriginX + (col + 0.5f) * kTileSize,
          kGridOriginY + (row + 0.5f) * kTileSize};
}

math::Vec2 SalonPlayState::cellCenterByIndex(int index) const {
  const int col = index % kGridCols;
  const int row = index / kGridCols;
  return cellCenter(col, row);
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
  case CellType::WaitSlot:
    return graphics::Color{0.20f, 0.40f, 0.55f, 1.0f};
  case CellType::Chair:
    return graphics::Color{0.40f, 0.28f, 0.42f, 1.0f};
  case CellType::Empty:
  default:
    return alternate ? graphics::Color{0.21f, 0.17f, 0.24f, 1.0f}
                     : graphics::Color{0.18f, 0.15f, 0.21f, 1.0f};
  }
}

// Customer helpers
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
      this->freeCustomerCell(customer);
      customer.setTargetPosition(cellCenter(kDoorCell.col, kDoorCell.row));
      customer.setCurrentState(CustomerState::Leaving);
    }
    break;

  case CustomerState::Queueing:
  case CustomerState::Leaving:
    break;
  }
}

void SalonPlayState::spawnCustomer() {
  const int waitSlotIndex = this->findFreeWaitSlot();
  if (waitSlotIndex < 0) {
    SAL_LOGW("Salon: no free wait slot, spawn rejected");
    this->toast_->setKind(ui::ToastKind::Warning)
        .setTitle("Warning!")
        .show("No free wait slot");
    return;
  }

  ++this->nextCustomerId_;

  const math::Vec2 spawnPos = cellCenter(kDoorCell.col, kDoorCell.row);
  const math::Vec2 waitPos = cellCenterByIndex(waitSlotIndex);

  Customer customer(this->nextCustomerId_, CustomerState::Arriving, spawnPos);
  customer.setTargetPosition(waitPos);
  customer.setReserveCellIndex(waitSlotIndex);

  this->reserveCell(waitSlotIndex, customer.id());
  this->customers_.push_back(customer);

  SAL_LOGI("Salon: spawning customer %u", this->nextCustomerId_);
}

bool SalonPlayState::seatCustomer(Customer &customer) {
  const int chairIndex = this->findFreeChair();
  if (chairIndex < 0) {
    SAL_LOGW("Salon: no free chair for customer: %u", customer.id());
    this->toast_->setKind(ui::ToastKind::Warning)
        .setTitle("Warning!")
        .show("No free chair");
    return false;
  }

  this->freeCustomerCell(customer);
  this->reserveCell(chairIndex, customer.id());

  customer.setReserveCellIndex(chairIndex);
  customer.setTargetPosition(cellCenterByIndex(chairIndex));
  customer.setCurrentState(CustomerState::WalkingToSeat);

  SAL_LOGI("Salon: customer %u -> chair %d", customer.id(), chairIndex);
  return true;
}

void SalonPlayState::removeGoneCustomers() {
  auto it = this->customers_.begin();
  while (it != this->customers_.end()) {
    if (it->currentState() == CustomerState::Leaving && it->hasArrived()) {
      this->freeCustomerCell(*it);
      it = this->customers_.erase(it);
    } else {
      ++it;
    }
  }
}

// Reservation helpers
int SalonPlayState::findFreeWaitSlot() const {
  for (const CellCoord &coord : kWaitSlots) {
    if (getCell(coord.col, coord.row).occupantId == 0) {
      return cellIndex(coord);
    }
  }
  return -1;
}

int SalonPlayState::findFreeChair() const {
  for (const CellCoord &coord : kChairSlots) {
    if (getCell(coord.col, coord.row).occupantId == 0) {
      return cellIndex(coord);
    }
  }
  return -1;
}

void SalonPlayState::reserveCell(int index, uint32_t customerId) {
  if (index < 0)
    return;
  this->cellByIndex(index).occupantId = customerId;
}

void SalonPlayState::freeCell(int index) {
  if (index < 0)
    return;
  cellByIndex(index).occupantId = 0;
}

void SalonPlayState::freeCustomerCell(Customer &customer) {
  if (customer.reservedCellIndex() < 0)
    return;
  this->freeCell(customer.reservedCellIndex());
  customer.setReserveCellIndex(-1);
}

} // namespace salshalon::features::salon
