#include "game/StateManager.hpp"

#include <utility>

#include "game/GameState.hpp"

namespace salshalon::game {

StateManager::StateManager() = default;
StateManager::~StateManager() = default;

void StateManager::change(std::unique_ptr<GameState> nextState) {
  this->pending_ = std::move(nextState);
  this->hasPending_ = true;
}

void StateManager::applyPending() {
  if (!this->hasPending_)
    return;
  this->hasPending_ = false;

  if (this->current_) {
    this->current_->onExit();
    this->current_.reset();
  }

  this->current_ = std::move(this->pending_);
  if (this->current_ != nullptr && this->engine_ != nullptr) {
    this->current_->onEnter(*this->engine_);
  }
}

} // namespace salshalon::game
