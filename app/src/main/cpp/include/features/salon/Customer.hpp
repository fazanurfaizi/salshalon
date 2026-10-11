#pragma once

#include "math/Math.hpp"
#include <cstdint>

namespace salshalon::features::salon {

enum class CustomerState {
  Arriving,
  Queueing,
  WalkingToSeat,
  Seated,
  Leaving,
};

struct StateTimer {
public:
  [[nodiscard]] float get(CustomerState state) const {
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
    }
  }

  void reset(CustomerState state) {
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
    }
  }

  void addTime(CustomerState state, float delta) {
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
    }
  }

private:
  float arriving_ = 0.0f;
  float queueing_ = 0.0f;
  float walkingToSeat_ = 0.0f;
  float seated_ = 0.0f;
  float leaving_ = 0.0f;
};

class Customer {
public:
  Customer(uint32_t id, CustomerState state, math::Vec2 spawnPosition);

  void update(float deltaSeconds);

  bool hasArrived() const;

  bool hasSatLongEnough() const;

  void setCurrentPosition(const math::Vec2 &position);

  void setTargetPosition(const math::Vec2 &position);

  void setCurrentState(CustomerState state);

  [[nodiscard]] const uint32_t id() const { return this->id_; }
  [[nodiscard]] const math::Vec2 currentPosition() const {
    return this->currentPosition_;
  }
  [[nodiscard]] const math::Vec2 targetPosition() const {
    return this->targetPosition_;
  }
  [[nodiscard]] const CustomerState currentState() const {
    return this->currentState_;
  }

private:
  static constexpr float kArrivalThreshold = 2.0f;
  static constexpr float kSeatedDuration = 3.0f;

  uint32_t id_;

  math::Vec2 currentPosition_{0.0f};
  math::Vec2 targetPosition_{0.0f};
  CustomerState currentState_{CustomerState::Arriving};
  StateTimer stateTimer_{};

  float movementSpeed_ = 200.0f;

  void moveTowardsTarget(float deltaSeconds);
};

} // namespace salshalon::features::salon
