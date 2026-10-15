#include "features/salon/SalonPlayState.hpp"

#include <cmath>
#include <glm/geometric.hpp>
#include <memory>
#include <random>
#include <string>

#include "audio/Sfx.hpp"
#include "core/Engine.hpp"
#include "core/GameConfig.hpp"
#include "core/Log.hpp"
#include "features/Feature.hpp"
#include "features/salon/CosmeticCatalog.hpp"
#include "features/salon/CosmeticUnlocks.hpp"
#include "features/salon/Customer.hpp"
#include "features/salon/CustomerCatalog.hpp"
#include "features/salon/DifficultyCurve.hpp"
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

  this->particles_.clear();

  this->engine_->audio().setAmbient(true);

  this->tutorialActive_ =
      !this->context_->tutorialSeen() && this->context_->day() == 1;
  this->tutorialStage_ = 0;
  this->tutorialTimer_ = 0.0f;

  SAL_LOGI("Salon: enter (day %d, money $%d, rep %d)", this->context_->day(),
           this->context_->money(), this->context_->reputation());
}

void SalonPlayState::onExit() {
  if (this->engine_ != nullptr)
    this->engine_->audio().setAmbient(false);
  this->context_->layout().resetOccupantId();
  this->assets_.unload();
  this->customers_.clear();
  this->particles_.clear();
  this->closeButton_.reset();
  this->toast_.reset();
  this->context_ = nullptr;
  SAL_LOGI("Salon: exit");
}

void SalonPlayState::update(float deltaSeconds) {
  this->elapsed_ += deltaSeconds;

  this->spawnTick(deltaSeconds);
  this->updateTutorial(deltaSeconds);

  this->closeButton_->update(deltaSeconds);
  this->toast_->update(deltaSeconds);
  this->particles_.update(deltaSeconds);

  for (auto &customer : this->customers_) {
    customer.update(deltaSeconds);
  }

  for (auto &customer : this->customers_) {
    this->applyCustomerTransition(customer);
  }

  this->removeGoneCustomers();

#ifndef NDEBUG
  this->debugLogTimer_ += deltaSeconds;
  if (this->debugLogTimer_ >= 2.0f) {
    this->debugLogTimer_ = 0.0f;
    SAL_LOGD("Diff: score=%u spawn=%.2fs patMult=%.2f rep=%d",
             this->sessionServed_, this->timeUntilNextSpawn_,
             DifficultyCurve::patienceMultiplier(sessionServed_),
             this->context_->reputation());
  }
#endif // !NDEBUG
}

void SalonPlayState::render(graphics::SpriteBatch &batch) {
  this->renderer_.setBatch(&batch);

  // Flat salon floor
  this->renderer_.drawRect(
      {0.0f, 0.0f, core::kDesignWidth, core::kDesignHeight},
      graphics::Color{0.18f, 0.13f, 0.10f, 1.0f});

  this->renderGrid();
  this->renderStations();
  this->renderDecor();
  this->renderCustomer();
  this->renderHud();
#ifndef NDEBUG
  this->renderDebugOverlay();
#endif // !NDEBUG

  this->particles_.render(batch);

  this->closeButton_->render(this->renderer_);

  this->toast_->render(this->renderer_,
                       {core::kDesignWidth * 0.5f, core::kDesignHeight - 70.0f},
                       ui::ToastAnchor::Center);
}

void SalonPlayState::renderGrid() {
  using L = SalonLayout;

  // Wall color as the background behind the grid.
  const auto *wallArch = CosmeticCatalog::find(this->context_->currentWallId());
  const graphics::Color wallColor =
      wallArch ? wallArch->primary : graphics::Color{0.30f, 0.26f, 0.22f, 1.0f};
  this->renderer_.drawRect({0.0f, 0.0f, kDesignWidth, kDesignHeight},
                           wallColor);

  // Floor colors.
  const auto *floorArch =
      CosmeticCatalog::find(this->context_->currentFloorId());
  const graphics::Color tileA =
      floorArch ? floorArch->primary
                : graphics::Color{0.28f, 0.20f, 0.14f, 1.0f};
  const graphics::Color tileB =
      floorArch ? floorArch->secondary
                : graphics::Color{0.32f, 0.23f, 0.16f, 1.0f};

  const math::Rect floorRect{
      L::kOriginX,
      L::kOriginY,
      L::kCols * L::kTileSize,
      L::kRows * L::kTileSize,
  };
  this->renderer_.drawRect(floorRect, tileA);

  const SalonLayout &layout = this->context_->layout();

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

      // Base tile: floor color for empty cells, cell type color otherwise.
      const graphics::Color baseColor = cellColor(cell.type, alternate);
      // const graphics::Color fill = (cell.type == CellType::Empty)
      //                                  ? (alternate ? tileB : tileA)
      //                                  : baseColor;
      graphics::Color fill;
      if (cell.type == CellType::Empty) {
        fill = alternate ? tileB : tileA;
      } else {
        fill = cellColor(cell.type, alternate);
      }
      this->renderer_.drawRect(cellRect.inset(1.0f), fill);

      if (cell.occupantId != 0) {
        this->renderer_.drawRect(cellRect.inset(L::kTileSize * 0.35f),
                                 graphics::Color{1.0f, 1.0f, 1.0f, 0.15f});
      }
    }
  }
}

void SalonPlayState::renderStations() {
  using L = SalonLayout;
  const SalonLayout &layout = this->context_->layout();

  for (const auto &inst : layout.stations()) {
    const math::Rect cellRect{
        L::kOriginX + inst.gridX * L::kTileSize,
        L::kOriginY + inst.gridY * L::kTileSize,
        L::kTileSize,
        L::kTileSize,
    };

    const StationArchetype *arch = StationCatalog::find(inst.archetypeId);
    const graphics::Color color =
        arch ? arch->placeholderColor : graphics::Color{0.7f, 0.7f, 0.7f, 1.0f};

    this->renderer_.drawRect(cellRect.inset(8.0f), color);
    this->renderer_.drawFrame(cellRect.inset(8.0f),
                              graphics::Color{1.0f, 1.0f, 1.0f, 0.30}, 2.0f);
  }
}

void SalonPlayState::renderDecor() {
  using L = SalonLayout;
  for (const auto &d : this->context_->layout().decors()) {
    const math::Rect cellRect{
        L::kOriginX + d.gridX * L::kTileSize,
        L::kOriginY + d.gridY * L::kTileSize,
        L::kTileSize,
        L::kTileSize,
    };

    const auto *arch = CosmeticCatalog::find(d.archetypeId);
    const graphics::Color c =
        arch ? arch->primary : graphics::Color{0.7f, 0.7f, 0.7f, 1.0f};

    this->renderer_.drawRect(cellRect.inset(L::kTileSize * 0.20f), c);
    this->renderer_.drawFrame(cellRect.inset(L::kTileSize * 0.20f),
                              graphics::Color{1.0f, 1.0f, 1.0f, 0.25f}, 2.0f);
  }
}

void SalonPlayState::renderCustomer() {
  // Placeholder customer
  for (const auto &customer : this->customers_) {
    graphics::Color bodyColor{0.7f, 0.7f, 0.7f, 1.0f};
    if (const auto *arch = CustomerCatalog::find(customer.archetypeId())) {
      bodyColor = arch->placeholderColor;
    }

    // Leaving customers fade out
    if (customer.currentState() == CustomerState::Leaving) {
      bodyColor.a *= 0.55f;
    }

    // Idle animation
    math::Vec2 center = customer.currentPosition();
    if (customer.currentState() == CustomerState::Queueing ||
        customer.currentState() == CustomerState::Seated) {
      const float phase = static_cast<float>(customer.id() % 7u) * 0.9f;
      center.y += std::sin(this->elapsed_ * 2.2f + phase) * 3.0f;
    }

    const math::Rect rect =
        math::Rect::fromCenter(center, kCustomerSize, kCustomerSize);
    this->renderer_.drawRect(rect, bodyColor);

    // Small head so orientation is readable
    const math::Rect head =
        math::Rect::fromCenter({center.x, center.y - kCustomerSize * 0.35f},
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

#ifndef NDEBUG
void SalonPlayState::renderDebugOverlay() {
  // Small debug panel at the top-left of the grid area.
  const float x = 24.0f;
  const float y = 80.0f;
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);

  const std::string line1 =
      "score=" + std::to_string(this->sessionServed_) +
      "  spawn=" + std::to_string(this->timeUntilNextSpawn_) + "s";

  const std::string line2 =
      "patMult=" +
      std::to_string(
          static_cast<int>(
              DifficultyCurve::patienceMultiplier(this->sessionServed_) * 100) /
          100.0f)
          .substr(0, 4) +
      "  rep=" + std::to_string(this->context_->reputation());

  const std::string line3 =
      "repSpawn x" +
      std::to_string(static_cast<int>(DifficultyCurve::reputationSpawnFactor(
                                          context_->reputation()) *
                                      100) /
                     100.0f)
          .substr(0, 4);

  this->renderer_.drawText(line1, {x, y}, 1.8f,
                           graphics::Color{0.85f, 1.0f, 0.85f, 0.75f});
  this->renderer_.drawText(line2, {x, y + 28.0f}, 1.8f,
                           graphics::Color{0.85f, 1.0f, 0.85f, 0.75f});
  this->renderer_.drawText(line3, {x, y + 56.0f}, 1.8f,
                           graphics::Color{0.85f, 1.0f, 0.85f, 0.75f});
}
#endif

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

const graphics::Texture *SalonPlayState::cellTexture(CellType type) const {
  switch (type) {
  case CellType::Door:
    return &this->assets_.door;
  case CellType::WaitSlot:
    return &this->assets_.chair;
  case CellType::Station:
    return nullptr;
  case CellType::Empty:
  default:
    return &this->assets_.floor;
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
      // Service complete: hearts for the happy customer + a small dust
      // puff where the station was used.
      this->particles_.spawn(customer.currentPosition(),
                             fx::ParticleKind::Heart, 8);
      this->particles_.spawn(customer.currentPosition(), fx::ParticleKind::Dust,
                             6);
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

  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  this->timeUntilNextSpawn_ = DifficultyCurve::jitteredSpawnInterval(
      this->sessionServed_, this->context_->reputation(), unit(this->rng_));
}

void SalonPlayState::spawnCustomer() {
  using namespace DifficultyCurve;

  const int waitSlotIndex = this->findFreeWaitSlot();
  if (waitSlotIndex < 0) {
    SAL_LOGW("Salon: no free wait slot, spawn rejected");
    this->toast_->setKind(ui::ToastKind::Warning)
        .setTitle("Warning!")
        .show("No free wait slot");
    return;
  }

  const CustomerArchetype *arch = CustomerCatalog::pickForReputation(
      this->context_->reputation(), this->rng_);
  if (arch == nullptr)
    return;

  ++this->nextCustomerId_;

  const float patience = kBasePatienceSeconds * arch->patienceMultiplier *
                         patienceMultiplier(this->sessionServed_);

  const math::Vec2 spawnPos = SalonLayout::cellCenter(
      SalonLayout::kDoorCell.col, SalonLayout::kDoorCell.row);
  const math::Vec2 waitPos = SalonLayout::cellCenterByIndex(waitSlotIndex);

  Customer customer(this->nextCustomerId_, CustomerState::Arriving, spawnPos,
                    arch->id, patience, arch->tipMultiplier);
  customer.setTargetPosition(waitPos);
  customer.setReserveCellIndex(waitSlotIndex);

  this->reserveCell(waitSlotIndex, customer.id());
  this->customers_.push_back(customer);

  if (this->engine_ != nullptr)
    this->engine_->audio().play(audio::Sfx::Doorbell);

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

  this->particles_.spawn(SalonLayout::cellCenterByIndex(stationIndex),
                         fx::ParticleKind::Dust, 10);
  if (this->engine_ != nullptr)
    this->engine_->audio().play(audio::Sfx::Tap);
  if (this->tutorialActive_ && this->tutorialStage_ == 1) {
    this->toast_->setKind(ui::ToastKind::Info)
        .setTitle("Seated!")
        .show("Service takes a moment — watch for coins");
    this->tutorialStage_ = 2;
  }

  SAL_LOGI("Salon: customer %u -> station %d", customer.id(), stationIndex);
  return true;
}

void SalonPlayState::removeGoneCustomers() {
  auto it = this->customers_.begin();
  while (it != this->customers_.end()) {
    if (it->currentState() == CustomerState::Leaving && it->hasArrived()) {
      this->freeCustomerCell(*it);
      switch (it->outcome()) {
      case CustomerOutcome::Served: {
        const int32_t payout = static_cast<int32_t>(
            DifficultyCurve::kBasePayout * it->tipMultiplier());
        this->context_->onCustomerServed(payout);
        ++this->sessionServed_;
        this->particles_.spawn(it->currentPosition(), fx::ParticleKind::Coin,
                               14);
        if (this->engine_ != nullptr) {
          this->engine_->addFlash(graphics::Color{1.0f, 0.9f, 0.45f, 0.35f},
                                  0.4f);
          this->engine_->audio().play(audio::Sfx::Served);
        }
        if (this->tutorialActive_ && this->tutorialStage_ == 2) {
          this->toast_->setKind(ui::ToastKind::Success)
              .setTitle("Payment collected!")
              .show("Coins earn money, happy customers earn reputation");
          this->tutorialStage_ = 3;
          this->markTutorialSeen();
        }
        break;
      }
      case CustomerOutcome::GaveUp:
        ++this->sessionLost_;
        this->context_->onCustomerLost();
        if (this->engine_ != nullptr) {
          this->engine_->addShake(0.35f, 10.0f);
          this->engine_->addFlash(graphics::Color{0.9f, 0.2f, 0.2f, 0.25f},
                                  0.35f);
        }
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

void SalonPlayState::updateTutorial(float deltaSeconds) {
  if (!this->tutorialActive_)
    return;
  this->tutorialTimer_ += deltaSeconds;
  if (this->tutorialStage_ == 0 && this->tutorialTimer_ > 0.8f) {
    this->toast_->setKind(ui::ToastKind::Info)
        .setTitle("Welcome!")
        .show("Tap a waiting customer to seat them");
    this->tutorialStage_ = 1;
  }
}

void SalonPlayState::markTutorialSeen() {
  if (!this->tutorialActive_)
    return;
  this->tutorialActive_ = false;
  this->tutorialStage_ = 3;
  if (this->context_ != nullptr) {
    this->context_->markTutorialSeen();
    if (this->engine_ != nullptr)
      this->engine_->saveProfile();
  }
}

void SalonPlayState::onClosePressed() {
  if (this->engine_ != nullptr)
    this->engine_->audio().play(audio::Sfx::Tap);
  this->markTutorialSeen();

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

  this->context_->recordDayResult(this->sessionServed_);
  this->context_->advanceDay();
  result.newlyUnlocked = CosmeticUnlocks::checkAndApply(*this->context_);
  this->engine_->saveProfile();

  goTo<SessionSummaryState>(*this->engine_, result);
}

} // namespace salshalon::features::salon
