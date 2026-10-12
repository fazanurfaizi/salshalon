#include "features/salon/SalonPlayState.hpp"

#include <glm/geometric.hpp>
#include <memory>
#include <string>

#include "core/Engine.hpp"
#include "core/GameConfig.hpp"
#include "core/Log.hpp"
#include "features/Feature.hpp"
#include "features/salon/Customer.hpp"
#include "features/salon/Session.hpp"
#include "features/salon/SessionSummaryState.hpp"
#include "features/salon/StationCatalog.hpp"
#include "graphics/Color.hpp"
#include "graphics/SpriteBatch.hpp"
#include "input/TouchEvent.hpp"
#include "math/Math.hpp"
#include "math/Rect.hpp"

namespace salshalon::features::salon {

using core::kDesignHeight;
using core::kDesignWidth;

void SalonPlayState::onEnter(core::Engine &engine) {
  game::GameState::onEnter(engine);
  this->context_ = &engine.salon();

  this->assets_.load(engine.assets());

  this->sessionServed_ = 0;
  this->sessionLost_ = 0;
  this->sessionMoneyBefore_ = this->context_->money();
  this->sessionReputationBefore_ = this->context_->reputation();
  this->timeUntilNextSpawn_ = kFirstSpawnDelaySeconds;

  const math::Rect closeBounds{
      kDesignWidth - 240.0f - 24.0f,
      kDesignHeight - kHudBottomHeight + 8.0f,
      240.0f,
      kHudBottomHeight - 16.0f,
  };
  this->closeButton_ = std::make_unique<ui::Button>(
      "CLOSE SALON", closeBounds, [this] { onClosePressed(); });
  this->closeButton_->setTextScale(3.0f);

  this->toast_ = std::make_unique<ui::Toast>();
  this->toast_->setKind(ui::ToastKind::Info).setFadeTime(1.5f);

  SAL_LOGI("Salon: enter (day %d, money $%d, rep %d)", this->context_->day(),
           this->context_->money(), this->context_->reputation());
}

void SalonPlayState::onExit() {
  this->context_->layout().resetOccupantId();
  this->assets_.unload();
  this->customers_.clear();
  this->closeButton_.reset();
  this->toast_.reset();
  this->context_ = nullptr;
  SAL_LOGI("Salon: exit");
}

void SalonPlayState::update(float deltaSeconds) {
  this->elapsed_ += deltaSeconds;

  this->spawnTick(deltaSeconds);

  this->closeButton_->update(deltaSeconds);
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

  this->closeButton_->render(this->renderer_);

  this->toast_->render(this->renderer_,
                       {core::kDesignWidth * 0.5f, core::kDesignHeight - 70.0f},
                       ui::ToastAnchor::Center);
}

void SalonPlayState::renderGrid() {
  using L = SalonLayout;

  // Floor background under the tiles
  const math::Rect floorRect{
      L::kOriginX,
      L::kOriginY,
      L::kCols * L::kTileSize,
      L::kRows * L::kTileSize,
  };
  this->renderer_.drawRect(floorRect,
                           graphics::Color{0.16f, 0.13f, 0.18f, 1.0f});

  const SalonLayout &layout = this->context_->layout();

  // Checkerboard tiles
  for (int row = 0; row < L::kRows; ++row) {
    for (int col = 0; col < L::kCols; ++col) {
      const math::Rect cellRect{
          L::kOriginX + col * L::kTileSize,
          L::kOriginY + row * L::kTileSize,
          L::kTileSize,
          L::kTileSize,
      };

      const bool alternate = ((col + row) & 1) != 0;
      const Cell &cell = layout.cellAt(col, row);

      const auto *station = layout.stationAt(col, row);
      if (station != nullptr) {
        const auto *archetype = StationCatalog::find(station->archetypeId);
        const graphics::Color color = (archetype != nullptr)
                                          ? archetype->placeholderColor
                                          : cellColor(cell.type, alternate);
        this->renderer_.drawRect(cellRect.inset(1.0f), color);
      } else {
        this->renderer_.drawRect(cellRect.inset(1.0f),
                                 cellColor(cell.type, alternate));
      }

      if (cell.occupantId != 0) {
        // @NOTE: fade marker on reserved cells. Remove when customer sprites
        this->renderer_.drawRect(cellRect.inset(L::kTileSize * 0.35f),
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

    if (customer.currentState() == CustomerState::Queueing) {
      this->renderPatienceBar(customer);
    }
  }
}

void SalonPlayState::renderPatienceBar(const Customer &customer) {
  const float remaining = 1.0f - customer.queueElapsedFraction();
  const float clamped = math::clampf(remaining, 0.0f, 1.0f);

  const float barX = customer.currentPosition().x - kCustomerSize * 0.5f;
  const float barY = customer.currentPosition().y - kCustomerSize * 0.5f -
                     kPatienceBarGap - kPatienceBarHeight;

  // Background track
  this->renderer_.drawRect({barX, barY, kCustomerSize, kPatienceBarHeight},
                           graphics::Color{0.15f, 0.15f, 0.18f, 0.9f});

  // Fill — red when almost gone, yellow mid, green when fresh.
  const graphics::Color fill =
      graphics::lerp(graphics::Color{0.90f, 0.25f, 0.25f, 1.0f},
                     graphics::Color{0.90f, 0.82f, 0.30f, 1.0f}, clamped);
  this->renderer_.drawRect(
      {barX, barY, kCustomerSize * clamped, kPatienceBarHeight}, fill);
}

void SalonPlayState::renderHud() {
  // Top
  this->renderer_.drawRect({0.0f, 0.0f, kDesignWidth, kHudTopHeight},
                           graphics::Color{0.04f, 0.04f, 0.07f, 1.0f});

  this->renderer_.drawText("DAY " + std::to_string(this->context_->day()),
                           {24.0f, kHudTopHeight * 0.5f - 12.0f}, 2.5f,
                           graphics::kWhite);

  // Reputation — center-aligned
  const std::string reputationStr =
      "REP:" + std::to_string(this->context_->reputation());
  const float reputationWidth =
      this->renderer_.measureText(reputationStr, 2.5f);
  this->renderer_.drawText(
      reputationStr,
      {kDesignWidth * 0.5 - reputationWidth, kHudTopHeight * 0.5f - 12.0f},
      2.5f, graphics::Color{0.85f, 1.0f, 0.75f, 1.0f});

  // Money — right-aligned
  const std::string moneyStr = "$" + std::to_string(this->context_->money());
  const float moneyWidth = this->renderer_.measureText(moneyStr, 2.5f);
  this->renderer_.drawText(
      moneyStr,
      {kDesignWidth - 24.0f - moneyWidth, kHudTopHeight * 0.5f - 12.0f}, 2.5f,
      graphics::Color{0.85f, 1.0f, 0.75f, 1.0f});

  // Bottom
  this->renderer_.drawRect(
      {0.0f, kDesignHeight - kHudBottomHeight, kDesignWidth, kHudBottomHeight},
      graphics::Color{0.04f, 0.04f, 0.07f, 1.0f});
}

void SalonPlayState::onTouch(const input::TouchEvent &event) {
  if (this->closeButton_ && this->closeButton_->onTouch(event))
    return;

  if (event.phase != input::TouchPhase::Down)
    return;

  if (event.phase == input::TouchPhase::Down) {
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

graphics::Color SalonPlayState::cellColor(CellType type, bool alternate) const {
  switch (type) {
  case CellType::Door:
    return graphics::Color{0.45f, 0.25f, 0.15f, 1.0f};
  case CellType::WaitSlot:
    return graphics::Color{0.20f, 0.40f, 0.55f, 1.0f};
  case CellType::Station:
    return graphics::Color{0.40f, 0.28f, 0.42f, 1.0f};
  case CellType::Empty:
  default:
    return alternate ? graphics::Color{0.21f, 0.17f, 0.24f, 1.0f}
                     : graphics::Color{0.18f, 0.15f, 0.21f, 1.0f};
  }
}

// Customer helpers
void SalonPlayState::applyCustomerTransition(Customer &customer) {
  using L = SalonLayout;
  switch (customer.currentState()) {
  case CustomerState::Arriving:
    if (customer.hasArrived()) {
      customer.setCurrentState(CustomerState::Queueing);
    }
    break;

  case CustomerState::Queueing:
    if (customer.hasGivenUp()) {
      this->freeCustomerCell(customer);
      customer.setTargetPosition(
          L::cellCenter(L::kDoorCell.col, L::kDoorCell.row));
      customer.setCurrentState(CustomerState::Leaving);
      customer.setOutcome(CustomerOutcome::GaveUp);
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
      customer.setTargetPosition(
          L::cellCenter(L::kDoorCell.col, L::kDoorCell.row));
      customer.setOutcome(CustomerOutcome::Served);
      customer.setCurrentState(CustomerState::Leaving);
    }
    break;

  case CustomerState::Leaving:
    break;
  }
}

void SalonPlayState::spawnTick(float deltaSeconds) {
  this->timeUntilNextSpawn_ -= deltaSeconds;
  if (timeUntilNextSpawn_ > 0.0f)
    return;

  const std::size_t before = this->customers_.size();
  this->spawnCustomer();
  if (this->customers_.size() == before)
    return;

  this->timeUntilNextSpawn_ = kSpawnIntervalSeconds;
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

  const math::Vec2 spawnPos = SalonLayout::cellCenter(
      SalonLayout::kDoorCell.col, SalonLayout::kDoorCell.row);
  const math::Vec2 waitPos = SalonLayout::cellCenterByIndex(waitSlotIndex);

  Customer customer(this->nextCustomerId_, CustomerState::Arriving, spawnPos);
  customer.setTargetPosition(waitPos);
  customer.setReserveCellIndex(waitSlotIndex);

  this->reserveCell(waitSlotIndex, customer.id());
  this->customers_.push_back(customer);

  SAL_LOGI("Salon: spawning customer %u", this->nextCustomerId_);
}

bool SalonPlayState::seatCustomer(Customer &customer) {
  const int stationIndex = this->findFreeStation();
  if (stationIndex < 0) {
    SAL_LOGW("Salon: no free chair for customer: %u", customer.id());
    this->toast_->setKind(ui::ToastKind::Warning)
        .setTitle("No free station!")
        .show("Buy more stations in build mode");
    return false;
  }

  this->freeCustomerCell(customer);
  this->reserveCell(stationIndex, customer.id());

  customer.setReserveCellIndex(stationIndex);
  customer.setTargetPosition(SalonLayout::cellCenterByIndex(stationIndex));
  customer.setCurrentState(CustomerState::WalkingToSeat);

  SAL_LOGI("Salon: customer %u -> station %d", customer.id(), stationIndex);
  return true;
}

void SalonPlayState::removeGoneCustomers() {
  auto it = this->customers_.begin();
  while (it != this->customers_.end()) {
    if (it->currentState() == CustomerState::Leaving && it->hasArrived()) {
      this->freeCustomerCell(*it);
      switch (it->outcome()) {
      case CustomerOutcome::Served:
        ++this->sessionServed_;
        this->context_->onCustomerServed(50);
        break;
      case CustomerOutcome::GaveUp:
        ++this->sessionLost_;
        this->context_->onCustomerLost();
        break;
      }
      it = this->customers_.erase(it);
    } else {
      ++it;
    }
  }
}

// Reservation helpers
int SalonPlayState::findFreeWaitSlot() const {
  using L = SalonLayout;
  const SalonLayout &layout = this->context_->layout();
  for (const auto &w : L::kWaitSlots) {
    if (layout.cellAt(w.col, w.row).occupantId == 0) {
      return L::cellIndex(w.col, w.row);
    }
  }
  return -1;
}

int SalonPlayState::findFreeStation() const {
  using L = SalonLayout;
  const SalonLayout &layout = this->context_->layout();
  for (const auto &s : layout.stations()) {
    if (layout.cellAt(s.gridX, s.gridY).occupantId == 0) {
      return L::cellIndex(s.gridX, s.gridY);
    }
  }
  return -1;
}

void SalonPlayState::reserveCell(int index, uint32_t customerId) {
  if (index < 0)
    return;
  this->context_->layout()
      .cellAt(index % SalonLayout::kCols, index / SalonLayout::kCols)
      .occupantId = customerId;
}

void SalonPlayState::freeCell(int index) {
  if (index < 0)
    return;
  this->context_->layout()
      .cellAt(index % SalonLayout::kCols, index / SalonLayout::kCols)
      .occupantId = 0;
}

void SalonPlayState::freeCustomerCell(Customer &customer) {
  if (customer.reservedCellIndex() < 0)
    return;
  this->freeCell(customer.reservedCellIndex());
  customer.setReserveCellIndex(-1);
}

void SalonPlayState::onClosePressed() {
  SessionResult result{
      .day = this->context_->day(),
      .moneyBefore = this->sessionMoneyBefore_,
      .moneyAfter = this->context_->money(),
      .reputationBefore = this->sessionReputationBefore_,
      .reputationAfter = this->context_->reputation(),
      .customersServed = this->sessionServed_,
      .customersLost = this->sessionLost_,
  };

  SAL_LOGI("Salon: closing day %d (served %u, lost %u)", result.day,
           result.customersServed, result.customersLost);

  this->context_->advanceDay();

  goTo<SessionSummaryState>(*this->engine_, result);
}

} // namespace salshalon::features::salon
