#pragma once

#include "features/salon/customers/CustomerLifecycle.hpp"

#include "math/Math.hpp"
#include <cstdint>
#include <string>

namespace salshalon::features::salon {

class Customer {
public:
  Customer(uint32_t id, CustomerState state, math::Vec2 spawnPosition,
           std::string archetypeId, float patienceSeconds, float tipMultiplier);

  void update(float deltaSeconds);

  bool hasArrived() const;
  bool hasSatLongEnough() const;
  bool hasGivenUp() const;
  [[nodiscard]] float queueElapsedFraction() const;

  void setCurrentPosition(const math::Vec2 &position);
  void setTargetPosition(const math::Vec2 &position);
  void setCurrentState(CustomerState state);
  void setOutcome(CustomerOutcome outcome);

  [[nodiscard]] uint32_t id() const { return this->id_; }
  [[nodiscard]] math::Vec2 currentPosition() const {
    return this->currentPosition_;
  }
  [[nodiscard]] math::Vec2 targetPosition() const {
    return this->targetPosition_;
  }
  [[nodiscard]] CustomerState currentState() const {
    return this->currentState_;
  }
  [[nodiscard]] CustomerOutcome outcome() const { return this->outcome_; }
  [[nodiscard]] const std::string &archetypeId() const {
    return this->archetypeId_;
  }
  [[nodiscard]] float tipMultiplier() const { return this->tipMultiplier_; }
  [[nodiscard]] float patienceSeconds() const { return this->patienceSeconds_; }

  [[nodiscard]] int reservedCellIndex() const {
    return this->reservedCellIndex_;
  }
  void setReserveCellIndex(int index);

private:
  static constexpr float kArrivalThreshold = 2.0f;
  static constexpr float kSeatedDuration = 3.0f;

  uint32_t id_;

  math::Vec2 currentPosition_{0.0f};
  math::Vec2 targetPosition_{0.0f};

  CustomerState currentState_{CustomerState::Arriving};
  CustomerOutcome outcome_{CustomerOutcome::GaveUp};
  StateTimer stateTimer_{};

  std::string archetypeId_;
  float patienceSeconds_ = 15.0f;
  float tipMultiplier_ = 1.0f;

  int reservedCellIndex_ = -1;

  float movementSpeed_ = 200.0f;

  void moveTowardsTarget(float deltaSeconds);
};

} // namespace salshalon::features::salon
