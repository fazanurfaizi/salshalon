#pragma once

#include <cstddef>

namespace salshalon::features::salon {

enum class CustomerState : std::size_t {
  Arriving,
  Queueing,
  WalkingToSeat,
  Seated,
  Leaving,
};

struct StateTimer {
public:
  [[nodiscard]] float get(CustomerState state) const;

  void reset(CustomerState state);

  void addTime(CustomerState state, float delta);

private:
  float arriving_ = 0.0f;
  float queueing_ = 0.0f;
  float walkingToSeat_ = 0.0f;
  float seated_ = 0.0f;
  float leaving_ = 0.0f;
};

} // namespace salshalon::features::salon
