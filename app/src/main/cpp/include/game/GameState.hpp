#pragma once

namespace salshalon::core {
class Engine;
}
namespace salshalon::graphics {
class SpriteBatch;
}
namespace salshalon::input {
struct TouchEvent;
}

namespace salshalon::game {

/// One screen of the game (menu, salon floor, results...),
class GameState {
public:
  virtual ~GameState() = default;

  /// Called once when the state becomes active.
  virtual void onEnter(core::Engine &engine) { this->engine_ = &engine; }

  /// Called once before the state is destroyed - release GPU resource.
  virtual void onExit() {}

  virtual void update(float deltaSeconds) = 0;
  virtual void render(graphics::SpriteBatch &batch) = 0;

  virtual void onTouch(const input::TouchEvent &event) { (void)event; }

protected:
  core::Engine *engine_ = nullptr;
};

} // namespace salshalon::game
