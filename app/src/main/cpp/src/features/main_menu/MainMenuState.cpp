#include "features/main_menu/MainMenuState.hpp"

#include <cmath>
#include <utility>

#include "core/Engine.hpp"
#include "core/GameConfig.hpp"
#include "core/Log.hpp"
#include "features/Feature.hpp"
#include "features/salon/SalonState.hpp"
#include "graphics/SpriteBatch.hpp"
#include "input/TouchEvent.hpp"
#include "ui/Layout.hpp"

namespace salshalon::features::main_menu {

namespace {
constexpr float kButtonWidth = 520.0f;
constexpr float kButtonHeight = 112.0f;
constexpr float kButtonGap = 40.0f;
constexpr float kFirstButtonY = 800.0f;

constexpr float kTitleScale = 8.0f;
constexpr float kSubtitleScale = 3.0f;
} // namespace

void MainMenuState::onEnter(core::Engine &engine) {
  GameState::onEnter(engine);
  this->assets_.load(engine.assets());
  buildUi();
}

void MainMenuState::onExit() {
  this->buttons_.clear();
  this->assets_.unload();
  SAL_LOGI("MainMenuState: exit");
}

void MainMenuState::buildUi() {
  this->buttons_.clear();

  ui::VerticalLayout layout{
      {0.0f, kFirstButtonY}, kButtonWidth, kButtonHeight, kButtonGap};

  auto addButton = [this](const char *label, const math::Rect &rect,
                          ui::Button::Callback callback) {
    auto button =
        std::make_unique<ui::Button>(label, rect, std::move(callback));
    button->setTextScale(4.0f);
    this->buttons_.push_back(std::move(button));
  };

  addButton("PLAY", layout.nextCentered(core::kDesignWidth),
            [this] { onPlayPressed(); });

  addButton("SETTINGS", layout.nextCentered(core::kDesignWidth),
            [this] { onSettingsPressed(); });

  addButton("QUIT", layout.nextCentered(core::kDesignWidth),
            [this] { onQuitPressed(); });
}

void MainMenuState::update(float deltaSeconds) {
  this->elapsed_ += deltaSeconds;

  for (auto &button : this->buttons_) {
    button->update(deltaSeconds);
  }

  if (this->toastTimer_ > 0.0f) {
    this->toastTimer_ -= deltaSeconds;
  }
}

void MainMenuState::render(graphics::SpriteBatch &batch) {
  this->renderer_.setBatch(&batch);

  // Background
  if (this->assets_.background.valid()) {
    batch.draw(this->assets_.background,
               {0.0f, 0.0f, core::kDesignWidth, core::kDesignHeight});
  } else {
    renderer_.drawRect({0.0f, 0.0f, core::kDesignWidth, core::kDesignHeight},
                       graphics::Color{0.06f, 0.07f, 0.13f, 1.0f});
  }

  // Subtle vertical bob.
  const float bob = std::sin(elapsed_ * 1.6f) * 6.0f;

  // Title
  const float titleCenterX = core::kDesignWidth * 0.5f;

  if (this->assets_.logo.valid()) {
    const float logoWidth = 560.0f;
    const float logoHeight = logoWidth *
                             static_cast<float>(this->assets_.logo.height()) /
                             static_cast<float>(this->assets_.logo.width());
    batch.draw(this->assets_.logo,
               math::Rect::fromCenter({titleCenterX, 340.0f + bob}, logoWidth,
                                      logoHeight));
  } else {
    this->renderer_.drawTextCentered("SALSHALON", {titleCenterX, 330.0f + bob},
                                     kTitleScale,
                                     graphics::Color{1.0f, 0.94f, 0.82f, 1.0f});
    this->renderer_.drawTextCentered(
        "salon rush", {titleCenterX, 430.0f + bob}, kSubtitleScale,
        graphics::Color{0.85f, 0.72f, 0.95f, 1.0f});
  }

  // Buttons
  for (auto &button : this->buttons_) {
    button->render(this->renderer_);
  }

  // Settings Overlay
  if (this->settingsOpen_) {
    this->renderer_.drawRect(
        {0.0f, 0.0f, core::kDesignWidth, core::kDesignHeight},
        graphics::Color{0.0f, 0.0f, 0.0f, 0.66f});

    const math::Rect panel = math::Rect::fromCenter(
        {core::kDesignWidth * 0.5f, core::kDesignHeight * 0.5f}, 580.0f,
        420.0f);

    this->renderer_.drawRect(panel, graphics::Color{0.14f, 0.16f, 0.23f, 1.0f});
    this->renderer_.drawFrame(panel, graphics::Color{1.0f, 1.0f, 1.0f, 0.3f},
                              3.0f);

    this->renderer_.drawTextCentered("SETTINGS",
                                     {panel.center().x, panel.y + 80.0f}, 4.5f,
                                     graphics::kWhite);
    this->renderer_.drawTextCentered(
        "coming soon", {panel.center().x, panel.center().y}, 3.0f,
        graphics::Color{0.80f, 0.86f, 0.94f, 1.0f});
    this->renderer_.drawTextCentered(
        "tap anywhere to close", {panel.center().x, panel.bottom() - 70.0f},
        2.2f, graphics::Color{0.58f, 0.63f, 0.72f, 1.0f});
  }

  // Toast
  if (this->toastTimer_ > 0.0f && !this->toast_.empty()) {
    const float alpha = math::clampf(this->toastTimer_, 0.0f, 1.0f);
    this->renderer_.drawTextCentered(
        this->toast_, {core::kDesignWidth * 0.5f, core::kDesignHeight - 70.0f},
        3.0f, graphics::Color{1.0f, 0.90f, 0.55f, alpha});
  }
}

void MainMenuState::onTouch(const input::TouchEvent &event) {
  // While the settings overlay is up it swallows every touch.
  if (this->settingsOpen_) {
    if (event.phase == input::TouchPhase::Up ||
        event.phase == input::TouchPhase::Down) {
      this->settingsOpen_ = false;
    }
    return;
  }

  for (auto &button : this->buttons_) {
    if (button->onTouch(event))
      break; // consumed
  }
}

void MainMenuState::showToast(std::string message, float seconds) {
  this->toast_ = std::move(message);
  this->toastTimer_ = seconds;
}

void MainMenuState::onPlayPressed() {
  SAL_LOGI("MainMenuState: PLAY pressed");
  showToast("\"SalonState\" is the next stop", 2.5f);
  goTo<salon::SalonState>(*this->engine_);
}

void MainMenuState::onSettingsPressed() {
  SAL_LOGI("MainMenuState: SETTINGS pressed");
  this->settingsOpen_ = true;
}

void MainMenuState::onQuitPressed() {
  SAL_LOGI("MainMenuState: QUIT pressed");
  if (this->engine_ != nullptr) {
    this->engine_->requestQuit();
  }
}

} // namespace salshalon::features::main_menu
