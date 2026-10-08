#include "features/settings/SettingsState.hpp"

#include "core/GameConfig.hpp"
#include "core/Log.hpp"
#include "features/Feature.hpp"
#include "graphics/SpriteBatch.hpp"
#include "input/TouchEvent.hpp"
#include "ui/Layout.hpp"

namespace salshalon::features::settings {

namespace {
constexpr float kButtonWidth = 520.0f;
constexpr float kButtonHeight = 112.0f;
constexpr float kButtonGap = 40.0f;
constexpr float kFirstButtonY = 800.0f;
} // namespace

void SettingsState::onEnter(core::Engine &engine) {
  GameState::onEnter(engine);
  buildUi();

  // Configure dialog
  this->settingsDialog_.setTitle("SETTINGS");
  this->settingsDialog_.setBody("coming soon");
  this->settingsDialog_.setHint("tap anywhere to close");

  // Configure toast
  this->toast_.setKind(ui::ToastKind::Info).setFadeTime(1.5f);
}

void SettingsState::onExit() {
  this->buttons_.clear();
  SAL_LOGI("SettingsState: exit");
}

void SettingsState::buildUi() {
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

  addButton("DIALOG", layout.nextCentered(core::kDesignWidth),
            [this] { onDialogPressed(); });

  addButton("SHOW", layout.nextCentered(core::kDesignWidth),
            [this] { onShowPressed(); });
}

void SettingsState::update(float deltaSeconds) {
  for (auto &button : this->buttons_) {
    button->update(deltaSeconds);
  }

  this->settingsDialog_.update(deltaSeconds);
  this->toast_.update(deltaSeconds);
}

void SettingsState::render(graphics::SpriteBatch &batch) {
  this->renderer_.setBatch(&batch);

  this->renderer_.drawRect(
      {0.0f, 0.0f, core::kDesignWidth, core::kDesignHeight},
      graphics::Color{0.10f, 0.11f, 0.15f, 1.0f});

  this->renderer_.drawTextCentered(
      "SETTINGS", {core::kDesignWidth * 0.5f, 240.0f}, 6.0f, graphics::kWhite);

  // Buttons
  for (auto &button : this->buttons_) {
    button->render(this->renderer_);
  }

  this->renderer_.drawTextCentered(
      "tap to go back",
      {core::kDesignWidth * 0.5f, core::kDesignHeight - 120.0f}, 2.5f,
      graphics::Color{0.7f, 0.7f, 0.7f, 1.0f});

  this->settingsDialog_.render(
      this->renderer_, {0.0f, 0.0f, core::kDesignWidth, core::kDesignHeight});

  this->toast_.render(this->renderer_,
                      {core::kDesignWidth * 0.5f, core::kDesignHeight - 70.0f},
                      ui::ToastAnchor::Center);
}

void SettingsState::onTouch(const input::TouchEvent &event) {
  if (this->settingsDialog_.onTouch(event))
    return;

  for (auto &button : this->buttons_) {
    if (button->onTouch(event))
      break; // consumed
  }
  // if (event.phase == input::TouchPhase::Up) {
  //   goTo<main_menu::MainMenuState>(*this->engine_);
  // }
}

void SettingsState::openSettingsDialog() {
  settingsDialog_.setTitle("SETTINGS");
  settingsDialog_.setBody("coming soon");
  settingsDialog_.setHint("tap anywhere to close");
  settingsDialog_.show();
}

void SettingsState::onDialogPressed() {
  SAL_LOGI("SettingsState: DIALOG pressed");
  this->openSettingsDialog();
}

void SettingsState::onShowPressed() {
  SAL_LOGI("SettingsState: SHOW pressed");
  this->toast_.setKind(ui::ToastKind::Warning)
      .setTitle("Warning!")
      .show("show toast");
  // goTo<settings::SettingsState>(*this->engine_);
}

} // namespace salshalon::features::settings
