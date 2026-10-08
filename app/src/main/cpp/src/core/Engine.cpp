#include "core/Engine.hpp"

#include <GLES2/gl2.h>
#include <game-activity/GameActivity.h>
#include <memory>

#include "core/GameConfig.hpp"
#include "core/Log.hpp"
#include "features/main_menu/MainMenuState.hpp"

namespace salshalon::core {

Engine::~Engine() { this->shutdown(); }

void Engine::init(GameActivity *activity, AAssetManager *assetManager) {
  this->activity_ = activity;
  this->assets_.setAssetManager(assetManager);
  this->states_.setEngine(this);
  this->camera_.setDesignResolution(kDesignWidth, kDesignHeight);
  this->input_.setTransform(this->camera_.transform());
  this->running_ = true;
}

void Engine::shutdown() {
  this->states_.change(nullptr);
  this->states_.applyPending();

  this->batch_.shutdown();
  this->gl_.destroy();

  this->initialized_ = false;
}

void Engine::onWindowCreated(ANativeWindow *window) {
  if (!this->gl_.init(window)) {
    SAL_LOGE("Engine: EGL initialization failed");
    return;
  }

  if (!this->batch_.init()) {
    SAL_LOGE("Engine: SpriteBatch initialization failed");
    this->gl_.destroy();
    return;
  }

  this->camera_.setDesignResolution(kDesignWidth, kDesignHeight);
  this->camera_.setSurfaceSize(this->gl_.width(), this->gl_.height());
  this->input_.setTransform(this->camera_.transform());

  this->clock.reset();
  this->initialized_ = true;

  // boot into the main menu
  this->states_.change(std::make_unique<features::main_menu::MainMenuState>());
  this->states_.applyPending();

  SAL_LOGI("Engine: window created (%dx%d)", gl_.width(), gl_.height());
}

void Engine::onWindowDestroyed() {
  this->states_.change(nullptr);
  this->states_.applyPending();

  this->batch_.shutdown();
  this->gl_.destroy();

  this->initialized_ = false;
  SAL_LOGI("Engine: window destroyed");
}

void Engine::onWindowResized() {
  if (this->initialized_)
    this->gl_.querySize();
}

void Engine::handleTouchRaw(const input::TouchEvent &event) {
  this->pendingTouches_.push_back(event);
}

void Engine::frame() {
  if (!this->initialized_ || !this->gl_.isValid())
    return;

  const float deltaSeconds = this->clock.tick();

  // Surface
  this->gl_.querySize();
  this->camera_.setSurfaceSize(this->gl_.width(), this->gl_.height());
  this->input_.setTransform(this->camera_.transform());

  // Input
  this->input_.beginFrame();
  for (const input::TouchEvent &raw : this->pendingTouches_) {
    this->input_.pushRawEvent(raw);
  }
  this->pendingTouches_.clear();

  // State
  this->states_.applyPending();

  game::GameState *state = this->states_.current();

  // Update
  if (state != nullptr) {
    for (const input::TouchEvent &event : this->input_.events()) {
      state->onTouch(event);
    }
    state->update(deltaSeconds);
  }

  // Render
  glViewport(0, 0, this->gl_.width(), this->gl_.height());
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  this->camera_.applyViewport();

  this->batch_.begin(this->camera_.projection());
  if (state != nullptr)
    state->render(this->batch_);
  this->batch_.end();

  this->gl_.swapBuffers();
}

void Engine::requestQuit() {
  SAL_LOGI("Engine: quit requested");
  this->running_ = false;

  if (this->activity_ != nullptr) {
    GameActivity_finish(this->activity_);
    // ANativeActivity_finish(this->activity_);
  }
}

} // namespace salshalon::core
