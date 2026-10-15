#pragma once

#include <memory>
#include <vector>

#include "audio/Sfx.hpp"
#include "core/AssetManager.hpp"
#include "core/Clock.hpp"
#include "core/GLContext.hpp"
#include "features/salon/SalonContext.hpp"
#include "game/StateManager.hpp"
#include "graphics/Camera2D.hpp"
#include "graphics/SpriteBatch.hpp"
#include "input/InputManager.hpp"

struct ANativeWindow;
struct GameActivity;
struct AAssetManager;

namespace salshalon::core {

/// Game runtime
class Engine {
public:
  Engine() = default;
  ~Engine();

  Engine(const Engine &) = delete;
  Engine &operator=(const Engine &) = delete;

  /// Called once from android_main, before event loop.
  void init(GameActivity *activity, AAssetManager *assetManager,
            const char *internalDataPath);

  void shutdown();

  /// Window lifecycle
  void onWindowCreated(ANativeWindow *window);
  void onWindowDestroyed();
  void onWindowResized();

  void handleTouchRaw(const input::TouchEvent &event);

  void frame();

  void requestQuit();

  bool running() const { return this->running_; }
  bool ready() const { return this->initialized_; }

  AssetManager &assets() { return assets_; }
  graphics::SpriteBatch &batch() { return batch_; }
  graphics::Camera2D &camera() { return camera_; }
  input::InputManager &input() { return input_; }
  game::StateManager &states() { return states_; }

  /// Shared, persistent salon state.
  features::salon::SalonContext &salon() { return *salon_; }

  /// Procedural sound effects + ambient music.
  audio::AudioEngine &audio() { return audio_; }

  /// Fullscreen game-feel feedback, driven per-frame in frame().
  /// Shake offsets the projection (decaying); flash draws a fading
  /// fullscreen tint after the state renders. Both are fire-and-forget.
  void addShake(float durationSeconds = 0.35f, float magnitudePx = 10.0f);
  void addFlash(const graphics::Color &color, float durationSeconds = 0.4f);

  void onPaused();
  bool saveProfile();

private:
  GameActivity *activity_ = nullptr;

  AssetManager assets_{};
  GLContext gl_{};
  graphics::Camera2D camera_{};
  graphics::SpriteBatch batch_{};
  input::InputManager input_{};
  game::StateManager states_{};
  Clock clock{};

  std::unique_ptr<features::salon::SalonContext> salon_{};
  std::string saveFilePath_{};

  std::vector<input::TouchEvent> pendingTouches_{};

  audio::AudioEngine audio_{};

  float fxTime_ = 0.0f;
  float shakeTime_ = 0.0f;
  float shakeDuration_ = 0.0f;
  float shakeMagnitude_ = 0.0f;
  graphics::Color flashColor_{1.0f, 1.0f, 1.0f, 0.0f};
  float flashTime_ = 0.0f;
  float flashDuration_ = 0.0f;

  bool initialized_ = false;
  bool running_ = false;
};

} // namespace salshalon::core
