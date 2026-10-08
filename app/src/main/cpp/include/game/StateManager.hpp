#pragma once

#include <memory>

namespace salshalon::core {
class Engine;
}

namespace salshalon::game {

class GameState;

class StateManager {
public:
  StateManager();
  ~StateManager();
  //
  void setEngine(core::Engine *engine) { this->engine_ = engine; }

  /// Queues a transition. pass nullptr to simply tear the current state down.
  void change(std::unique_ptr<GameState> nextState);

  /// Applies a queued transition. Called by engine at the top of each frame.
  void applyPending();

  GameState *current() const { return this->current_.get(); }

private:
  core::Engine *engine_ = nullptr;
  std::unique_ptr<GameState> current_{};
  std::unique_ptr<GameState> pending_{};
  bool hasPending_ = false;
};

} // namespace salshalon::game
