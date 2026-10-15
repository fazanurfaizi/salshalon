#include "core/Engine.hpp"

#include <GLES2/gl2.h>
#include <algorithm>
#include <cmath>
#include <game-activity/GameActivity.h>
#include <memory>

#include "core/GameConfig.hpp"
#include "core/Log.hpp"
#include "features/main_menu/MainMenuState.hpp"
#include "features/salon/SalonContext.hpp"
#include "features/salon/SaveSystem.hpp"

namespace salshalon::core {

Engine::~Engine() { this->shutdown(); }

void Engine::init(GameActivity *activity, AAssetManager *assetManager,
                  const char *internalDataPath) {
  this->activity_ = activity;
  this->assets_.setAssetManager(assetManager);
  this->states_.setEngine(this);
  this->camera_.setDesignResolution(kDesignWidth, kDesignHeight);
  this->input_.setTransform(this->camera_.transform());

  this->salon_ = std::make_unique<features::salon::SalonContext>();
  if (internalDataPath != nullptr && internalDataPath[0] != '\0') {
    this->saveFilePath_ =
        features::salon::SaveSystem::profilePath(internalDataPath);
    const auto loaded = features::salon::SaveSystem::load(this->saveFilePath_);
    if (loaded) {
      this->salon_->applyProfile(*loaded);
    } else {
      SAL_LOGI("Engine: no save found, using default profile");
    }
  } else {
    SAL_LOGW("Engine: no internal data path — saves disabled");
  }

  this->running_ = true;
}

void Engine::shutdown() {
  this->states_.change(nullptr);
  this->states_.applyPending();

  this->audio_.shutdown();
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

  this->fxTime_ += deltaSeconds;
  if (this->shakeTime_ > 0.0f)
    this->shakeTime_ = std::max(0.0f, this->shakeTime_ - deltaSeconds);
  if (this->flashTime_ > 0.0f)
    this->flashTime_ = std::max(0.0f, this->flashTime_ - deltaSeconds);
  this->audio_.update();

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

  math::Mat4 projection = this->camera_.projection();
  if (this->shakeTime_ > 0.0f && this->shakeDuration_ > 0.0f) {
    const float t = this->shakeTime_ / this->shakeDuration_;
    const float mag = this->shakeMagnitude_ * t;
    const float ox = std::sin(this->fxTime_ * 70.0f) * mag;
    const float oy = std::cos(this->fxTime_ * 53.0f) * mag;
    projection = glm::translate(projection, math::Vec3(ox, oy, 0.0f));
  }

  this->batch_.begin(projection);
  if (state != nullptr)
    state->render(this->batch_);

  if (this->flashTime_ > 0.0f && this->flashDuration_ > 0.0f) {
    const float t = this->flashTime_ / this->flashDuration_;
    const graphics::Color c{this->flashColor_.r, this->flashColor_.g,
                            this->flashColor_.b, this->flashColor_.a * t};
    this->batch_.drawRect(this->camera_.worldBounds(), c);
  }
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

bool Engine::saveProfile() {
  if (this->saveFilePath_.empty() || !this->salon_) {
    SAL_LOGW("Engine::saveProfile: no path configured");
    return false;
  }
  return features::salon::SaveSystem::save(this->saveFilePath_,
                                           this->salon_->toProfile());
}

void Engine::addShake(float durationSeconds, float magnitudePx) {
  if (durationSeconds <= 0.0f || magnitudePx <= 0.0f)
    return;
  this->shakeTime_ = durationSeconds;
  this->shakeDuration_ = durationSeconds;
  this->shakeMagnitude_ = magnitudePx;
}

void Engine::addFlash(const graphics::Color &color, float durationSeconds) {
  if (durationSeconds <= 0.0f)
    return;
  this->flashColor_ = color;
  this->flashTime_ = durationSeconds;
  this->flashDuration_ = durationSeconds;
}

void Engine::onPaused() {
  if (this->salon_ && !this->saveFilePath_.empty()) {
    if (this->saveProfile())
      SAL_LOGI("Engine: progress saved on pause");
  }
}

} // namespace salshalon::core
