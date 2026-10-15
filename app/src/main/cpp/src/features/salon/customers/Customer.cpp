#include "features/salon/customers/Customer.hpp"

#include <algorithm>
#include <glm/geometric.hpp>

#include "core/Log.hpp"

namespace salshalon::features::salon {

Customer::Customer(uint32_t id, CustomerState state, math::Vec2 spawnPosition,
                   std::string archetypeId, float patienceSeconds,
                   float tipMultiplier)
    : id_(id), currentPosition_(spawnPosition), targetPosition_(spawnPosition),
      currentState_(state), archetypeId_(std::move(archetypeId)),
      patienceSeconds_(patienceSeconds), tipMultiplier_(tipMultiplier) {}

void Customer::update(float deltaSeconds) {
  this->stateTimer_.addTime(this->currentState_, deltaSeconds);

  switch (this->currentState_) {
  case CustomerState::Arriving:
  case CustomerState::WalkingToSeat:
  case CustomerState::Leaving:
    this->moveTowardsTarget(deltaSeconds);
    break;
  case CustomerState::Queueing:
    break;
  case CustomerState::Seated:
    break;
  }
}

bool Customer::hasArrived() const {
  return glm::distance(this->currentPosition_, this->targetPosition_) <
         kArrivalThreshold;
}

bool Customer::hasSatLongEnough() const {
  return this->stateTimer_.get(CustomerState::Seated) >= kSeatedDuration;
}

bool Customer::hasGivenUp() const {
  return this->stateTimer_.get(CustomerState::Queueing) >=
         this->patienceSeconds_;
}

float Customer::queueElapsedFraction() const {
  if (this->patienceSeconds_ <= 0.0f)
    return 1.0f;
  return this->stateTimer_.get(CustomerState::Queueing) /
         this->patienceSeconds_;
}

void Customer::setCurrentPosition(const math::Vec2 &position) {
  currentPosition_ = position;
}

void Customer::setTargetPosition(const math::Vec2 &position) {
  targetPosition_ = position;
}

void Customer::setOutcome(CustomerOutcome outcome) { this->outcome_ = outcome; }

void Customer::setCurrentState(CustomerState state) {
  if (state == currentState_)
    return;

  SAL_LOGD("Customer %u: state %d -> %d", id_, static_cast<int>(currentState_),
           static_cast<int>(state));

  currentState_ = state;

  this->stateTimer_.reset(state);
}

void Customer::setReserveCellIndex(int index) {
  this->reservedCellIndex_ = index;
}

void Customer::moveTowardsTarget(float deltaSeconds) {
  const math::Vec2 delta = this->targetPosition_ - this->currentPosition_;
  const float dist = glm::length(delta);
  if (dist < 1.0f) {
    this->currentPosition_ = targetPosition_;
    return;
  }
  const float step = std::min(this->movementSpeed_ * deltaSeconds, dist);
  this->currentPosition_ += (delta / dist) * step;
}

} // namespace salshalon::features::salon
