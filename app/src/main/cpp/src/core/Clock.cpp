#include "core/Clock.hpp"

#include <algorithm>

namespace salshalon::core {

float Clock::tick() {
  const auto now = std::chrono::steady_clock::now();
  const std::chrono::duration<float> delta = now - this->last_;
  this->last_ = now;
  return std::min(delta.count(), this->maxDelta_);
}

} // namespace salshalon::core
