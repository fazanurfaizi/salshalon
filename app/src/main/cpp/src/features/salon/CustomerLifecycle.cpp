#include "features/salon/CustomerLifecycle.hpp"
#include "core/Log.hpp"
#include <cassert>

namespace salshalon::features::salon {

[[nodiscard]] float StateTimer::get(CustomerState state) const {
  switch (state) {
  case CustomerState::Arriving:
    return this->arriving_;
  case CustomerState::Queueing:
    return this->queueing_;
  case CustomerState::WalkingToSeat:
    return this->walkingToSeat_;
  case CustomerState::Seated:
    return this->seated_;
  case CustomerState::Leaving:
    return this->leaving_;
  default:
    SAL_LOGE("StateTimer::get: unhandled state %d", static_cast<int>(state));
    return 0.0f;
  }
}

void StateTimer::reset(CustomerState state) {
  switch (state) {
  case CustomerState::Arriving:
    this->arriving_ = 0.0f;
    break;
  case CustomerState::Queueing:
    this->queueing_ = 0.0f;
    break;
  case CustomerState::WalkingToSeat:
    this->walkingToSeat_ = 0.0f;
    break;
  case CustomerState::Seated:
    this->seated_ = 0.0f;
    break;
  case CustomerState::Leaving:
    this->leaving_ = 0.0f;
    break;
  default:
    SAL_LOGE("StateTimer::reset: unhandled state %d", static_cast<int>(state));
    break;
  }
}

void StateTimer::addTime(CustomerState state, float delta) {
  switch (state) {
  case CustomerState::Arriving:
    this->arriving_ += delta;
    break;
  case CustomerState::Queueing:
    this->queueing_ += delta;
    break;
  case CustomerState::WalkingToSeat:
    this->walkingToSeat_ += delta;
    break;
  case CustomerState::Seated:
    this->seated_ += delta;
    break;
  case CustomerState::Leaving:
    this->leaving_ += delta;
    break;
  default:
    SAL_LOGE("StateTimer::addTime: unhandled state %d",
             static_cast<int>(state));
    break;
  }
}

} // namespace salshalon::features::salon
