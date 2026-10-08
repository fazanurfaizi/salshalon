#pragma once

#include <vector>

#include "core/AssetManager.hpp"
#include "core/Clock.hpp"
#include "core/GLContext.hpp"
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
  void init(GameActivity *activity, AAssetManager *assetManager);

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

private:
  GameActivity *activity_ = nullptr;

  AssetManager assets_{};
  GLContext gl_{};
  graphics::Camera2D camera_{};
  graphics::SpriteBatch batch_{};
  input::InputManager input_{};
  game::StateManager states_{};
  Clock clock{};

  std::vector<input::TouchEvent> pendingTouches_{};

  bool initialized_ = false;
  bool running_ = false;
};

} // namespace salshalon::core
