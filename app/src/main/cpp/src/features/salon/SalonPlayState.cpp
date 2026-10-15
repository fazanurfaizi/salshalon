#include "features/salon/SalonPlayState.hpp"

#include <cmath>
#include <memory>
#include <string>

#include "audio/Sfx.hpp"
#include "core/Engine.hpp"
#include "core/GameConfig.hpp"
#include "core/Log.hpp"
#include "features/Feature.hpp"
#include "features/salon/DifficultyCurve.hpp"
#include "features/salon/Session.hpp"
#include "features/salon/SessionSummaryState.hpp"
#include "features/salon/cosmetics/CosmeticUnlocks.hpp"
#include "features/salon/customers/CustomerCatalog.hpp"
#include "features/salon/shared/SalonRenderer.hpp"
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
  this->manager_.reset();

  this->sessionMoneyBefore_ = this->context_->money();
  this->sessionReputationBefore_ = this->context_->reputation();

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
  this->tutorial_.begin(!this->context_->tutorialSeen() &&
                        this->context_->day() == 1);

  SAL_LOGI("Salon: enter (day %d, money $%d, rep %d)", this->context_->day(),
           this->context_->money(), this->context_->reputation());
}

void SalonPlayState::onExit() {
  if (this->engine_ != nullptr)
    this->engine_->audio().setAmbient(false);
  this->context_->layout().resetOccupantId();
  this->assets_.unload();
  this->manager_.reset();
  this->particles_.clear();
  this->closeButton_.reset();
  this->toast_.reset();
  this->context_ = nullptr;
  SAL_LOGI("Salon: exit");
}

void SalonPlayState::update(float deltaSeconds) {
  this->manager_.update(deltaSeconds, *this->context_, this->events_);
  this->consumeEvents();
  this->tutorial_.update(deltaSeconds, *this->toast_);

  this->closeButton_->update(deltaSeconds);
  this->toast_->update(deltaSeconds);
  this->particles_.update(deltaSeconds);

#ifndef NDEBUG
  this->debugLogTimer_ += deltaSeconds;
  if (this->debugLogTimer_ >= 2.0f) {
    this->debugLogTimer_ = 0.0f;
    SAL_LOGD("Diff: score=%u patMult=%.2f rep=%d", this->manager_.served(),
             DifficultyCurve::patienceMultiplier(this->manager_.served()),
             this->context_->reputation());
  }
#endif // !NDEBUG
}

void SalonPlayState::consumeEvents() {
  if (this->events_.spawnRejected) {
    this->toast_->setKind(ui::ToastKind::Warning)
        .setTitle("Warning!")
        .show("No free wait slot");
  }
  if (this->events_.spawned > 0 && this->engine_ != nullptr) {
    this->engine_->audio().play(audio::Sfx::Doorbell);
  }
  for (const math::Vec2 &pos : this->events_.completedAt) {
    // Service complete
    this->particles_.spawn(pos, fx::ParticleKind::Heart, 8);
    this->particles_.spawn(pos, fx::ParticleKind::Dust, 6);
  }
  for (const math::Vec2 &pos : this->events_.servedAt) {
    // Payment: coin burst, bright pulse, completion chime.
    this->particles_.spawn(pos, fx::ParticleKind::Coin, 14);
    if (this->engine_ != nullptr) {
      this->engine_->addFlash(graphics::Color{1.0f, 0.9f, 0.45f, 0.35f}, 0.4f);
      this->engine_->audio().play(audio::Sfx::Served);
    }
    if (this->tutorial_.onFirstPayment(*this->toast_))
      this->completeTutorial();
  }
  if (this->events_.gaveUp > 0 && this->engine_ != nullptr) {
    // Customer lost: subtle shake + red edge pulse.
    this->engine_->addShake(0.35f, 10.0f);
    this->engine_->addFlash(graphics::Color{0.9f, 0.2f, 0.2f, 0.25f}, 0.35f);
  }
}

void SalonPlayState::render(graphics::SpriteBatch &batch) {
  this->renderer_.setBatch(&batch);

  SalonRenderer::renderRoom(this->renderer_, *this->context_,
                            /*showOccupants=*/true);
  this->renderCustomer();
  this->particles_.render(batch);
  this->renderHud();
#ifndef NDEBUG
  this->renderDebugOverlay();
#endif // !NDEBUG

  this->closeButton_->render(this->renderer_);

  this->toast_->render(this->renderer_,
                       {core::kDesignWidth * 0.5f, core::kDesignHeight - 70.0f},
                       ui::ToastAnchor::Center);
}

void SalonPlayState::renderCustomer() {
  for (const auto &customer : this->manager_.customers()) {
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
      center.y += std::sin(this->manager_.elapsed() * 2.2f + phase) * 3.0f;
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

  const std::string line1 =
      "score=" + std::to_string(this->manager_.served()) +
      "  customers=" + std::to_string(this->manager_.customers().size());

  const std::string line2 =
      "patMult=" +
      std::to_string(static_cast<int>(DifficultyCurve::patienceMultiplier(
                                          this->manager_.served()) *
                                      100) /
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

  math::Vec2 seatPos{0.0f, 0.0f};
  switch (this->manager_.trySeatAt(event.position, *this->context_, seatPos)) {
  case SeatResult::Seated:
    // Station use: dust puff at the station + tap confirmation.
    this->particles_.spawn(seatPos, fx::ParticleKind::Dust, 10);
    if (this->engine_ != nullptr)
      this->engine_->audio().play(audio::Sfx::Tap);
    this->tutorial_.onSeated(*this->toast_);
    break;
  case SeatResult::NoStation:
    this->toast_->setKind(ui::ToastKind::Warning)
        .setTitle("No free station!")
        .show("Buy more stations in build mode");
    break;
  case SeatResult::NoCustomer:
    break;
  }
}

void SalonPlayState::completeTutorial() {
  if (!this->tutorial_.active())
    return;
  this->tutorial_.finish();
  if (this->context_ != nullptr) {
    this->context_->markTutorialSeen();
    if (this->engine_ != nullptr)
      this->engine_->saveProfile();
  }
}

void SalonPlayState::onClosePressed() {
  if (this->engine_ != nullptr)
    this->engine_->audio().play(audio::Sfx::Tap);
  this->completeTutorial();

  SessionResult result{
      .day = this->context_->day(),
      .moneyBefore = this->sessionMoneyBefore_,
      .moneyAfter = this->context_->money(),
      .reputationBefore = this->sessionReputationBefore_,
      .reputationAfter = this->context_->reputation(),
      .customersServed = this->manager_.served(),
      .customersLost = this->manager_.lost(),
  };

  SAL_LOGI("Salon: closing day %d (served %u, lost %u)", result.day,
           result.customersServed, result.customersLost);

  this->context_->recordDayResult(this->manager_.served());
  this->context_->advanceDay();
  result.newlyUnlocked = CosmeticUnlocks::checkAndApply(*this->context_);
  this->engine_->saveProfile();

  goTo<SessionSummaryState>(*this->engine_, result);
}

} // namespace salshalon::features::salon
