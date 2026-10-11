#pragma once

#include "features/salon/CustomerState.hpp"

#include "math/Math.hpp"
#include <cstdint>

namespace salshalon::features::salon {

class Customer {
public:
  Customer(uint32_t id, CustomerState state, math::Vec2 spawnPosition);

  void update(float deltaSeconds);

  bool hasArrived() const;

  bool hasSatLongEnough() const;

  void setCurrentPosition(const math::Vec2 &position);

  void setTargetPosition(const math::Vec2 &position);

  void setCurrentState(CustomerState state);

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
  StateTimer stateTimer_{};

  int reservedCellIndex_ = -1;

  float movementSpeed_ = 200.0f;

  void moveTowardsTarget(float deltaSeconds);
};

} // namespace salshalon::features::salon
