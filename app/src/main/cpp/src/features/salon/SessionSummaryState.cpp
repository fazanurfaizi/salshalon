#include "features/salon/SessionSummaryState.hpp"

#include "core/GameConfig.hpp"
#include "core/Log.hpp"
#include "features/Feature.hpp"
#include "features/main_menu/MainMenuState.hpp"
#include <string>

namespace salshalon::features::salon {

using core::kDesignHeight;
using core::kDesignWidth;

namespace {
constexpr float kPanelWidth = 700.0f;
constexpr float kPanelHeight = 420.0f;
constexpr float kRowHeight = 56.0f;
constexpr float kFirstRowY = 100.0f;
constexpr float kLabelScale = 3.4f;
constexpr float kValueScale = 3.4f;
constexpr float kTitleScale = 5.5;

constexpr float kButtonWidth = 400.0f;
constexpr float kButtonHeight = 96.0f;
} // namespace

SessionSummaryState::SessionSummaryState(SessionResult result)
    : result_(result) {}

void SessionSummaryState::onEnter(core::Engine &engine) {
  game::GameState::onEnter(engine);
  this->buildUi();
  SAL_LOGI("Summary: day %d — served %u, lost %u, money %d, rep %d",
           result_.day, result_.customersServed, result_.customersLost,
           result_.moneyEarned(), result_.reputationDelta());
}

void SessionSummaryState::onExit() { this->continueButton_.reset(); }

void SessionSummaryState::update(float deltaSeconds) {
  this->continueButton_->update(deltaSeconds);
}

void SessionSummaryState::onTouch(const input::TouchEvent &event) {
  this->continueButton_->onTouch(event);
}

void SessionSummaryState::render(graphics::SpriteBatch &batch) {
  this->renderer_.setBatch(&batch);

  // Backdrop
  this->renderer_.drawRect({0.f, 0.0f, kDesignWidth, kDesignHeight},
                           graphics::Color{0.06f, 0.07f, 0.13f, 1.0f});

  // Panel
  const math::Rect panel = math::Rect::fromCenter(
      {kDesignWidth * 0.5f, kDesignHeight * 0.5f}, kPanelWidth, kPanelHeight);
  this->renderer_.drawRect(panel, graphics::Color{0.14f, 0.16f, 0.23f, 1.0f});
  this->renderer_.drawFrame(panel, graphics::Color{1.0f, 1.0f, 1.0f, 0.30f},
                            3.0f);

  // Title
  this->renderer_.drawTextCentered(
      "DAY" + std::to_string(this->result_.day) + " COMPLETE",
      {panel.center().x, panel.y + 60.0f}, kTitleScale, graphics::kWhite);

  // Rows — label left-aligned, value right-aligned, both inside the panel.
  const float leftX = panel.x + 60.0f;
  const float rightX = panel.right() - 60.0f;
  float cursorY = panel.y + kFirstRowY;

  auto drawRow = [&](const std::string &label, const std::string &value,
                     const graphics::Color &valueColor) {
    this->renderer_.drawText(label, {leftX, cursorY}, kLabelScale,
                             graphics::Color{0.75f, 0.80f, 0.88f, 1.0f});

    const float valueWidth = this->renderer_.measureText(value, kValueScale);
    this->renderer_.drawText(value, {rightX - valueWidth, cursorY}, kValueScale,
                             valueColor);

    cursorY += kRowHeight;
  };

  drawRow("Customers served", std::to_string(this->result_.customersServed),
          graphics::Color{0.85f, 1.0f, 0.75f, 1.0f});

  drawRow("Customers lost", std::to_string(this->result_.customersLost),
          this->result_.customersLost > 0
              ? graphics::Color{1.0f, 0.55f, 0.55f, 1.0f}
              : graphics::Color{0.75f, 0.80f, 0.88f, 1.0f});

  const int32_t money = this->result_.moneyEarned();
  const std::string moneyValue =
      (money >= 0 ? "+$" : "-$") + std::to_string(money >= 0 ? money : -money);
  drawRow("Money earned", moneyValue,
          money >= 0 ? graphics::Color{0.85f, 1.0f, 0.75f, 1.0f}
                     : graphics::Color{1.0f, 0.55f, 0.55f, 1.0f});

  const int32_t repDelta = this->result_.reputationDelta();
  const std::string repValue =
      std::to_string(this->result_.reputationBefore) + " -> " +
      std::to_string(this->result_.reputationAfter) + "  (" +
      (repDelta >= 0 ? "+" : "") + std::to_string(repDelta) + ")";
  drawRow("Reputation", repValue,
          repDelta >= 0 ? graphics::Color{0.85f, 1.0f, 0.75f, 1.0f}
                        : graphics::Color{1.0f, 0.55f, 0.55f, 1.0f});

  // Continue button
  this->continueButton_->render(this->renderer_);
}

void SessionSummaryState::buildUi() {
  const float buttonY = kDesignHeight * 0.5f + kPanelHeight * 0.5f + 40.0f;
  this->continueButton_ = std::make_unique<ui::Button>(
      "CONTINUE",
      math::Rect::fromCenter({kDesignWidth * 0.5f, buttonY}, kButtonWidth,
                             kButtonHeight),
      [this] { this->onContinuePressed(); });
  this->continueButton_->setTextScale(4.0f);
}

void SessionSummaryState::onContinuePressed() {
  SAL_LOGI("Summary: continue");
  goTo<main_menu::MainMenuState>(*this->engine_);
}

} // namespace salshalon::features::salon
