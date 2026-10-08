#pragma once

#include <chrono>

namespace salshalon::core {

class Clock {
public:
  Clock() { this->reset(); }

  void reset() { this->last_ = std::chrono::steady_clock::now(); }

  float tick();

  void setMaxDelta(float seconds) { this->maxDelta_ = seconds; }

private:
  std::chrono::steady_clock::time_point last_;
  float maxDelta_ = 0.1;
}; // namespace salshalon::core

} // namespace salshalon::core
