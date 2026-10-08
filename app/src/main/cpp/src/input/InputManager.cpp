#include "input/InputManager.hpp"

namespace salshalon::input {

void InputManager::beginFrame() {
  this->events_.clear();
  this->justPressed_ = false;
  this->justReleased_ = false;
}

void InputManager::pushRawEvent(const TouchEvent &rawEvent) {
  TouchEvent event = rawEvent;
  event.position = this->transform_.toWorld(rawEvent.position);

  switch (event.phase) {
  case TouchPhase::Down:
    if (this->activePointer_ == -1) {
      this->activePointer_ = event.pointerId;
      this->down_ = true;
      this->justPressed_ = true;
    } else if (event.pointerId != this->activePointer_) {
      return;
    }
    break;
  case TouchPhase::Move:
    if (event.pointerId != this->activePointer_)
      return;
    break;
  case TouchPhase::Up:
  case TouchPhase::Cancel:
    if (event.pointerId != this->activePointer_)
      return;
    this->activePointer_ = -1;
    this->down_ = false;
    this->justReleased_ = true;
    break;
  }

  this->worldPosition_ = event.position;
  this->events_.push_back(event);
}

} // namespace salshalon::input
