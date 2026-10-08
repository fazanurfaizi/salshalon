#include "features/salon/SalonContext.hpp"

#include <algorithm>

#include "core/Log.hpp"

namespace salshalon::features::salon {

bool SalonContext::earn(int32_t amount) {
  if (amount < 0) {
    SAL_LOGW("SalonContext::earn: negative amount (%d), ignored", amount);
    return false;
  }
  this->money_ += amount;
  return true;
}

bool SalonContext::trySpend(int32_t amount) {
  if (amount <= 0) {
    SAL_LOGW("SalonContext::trySpend: non-positive amount (%d)", amount);
    return false;
  }
  if (amount > this->money_) {
    SAL_LOGW("SalonContext::trySpend: not enough money (%d)", amount);
    return false;
  }
  this->money_ -= amount;
  return true;
}

void SalonContext::adjustReputation(int32_t delta) {
  const int32_t before = this->reputation_;
  this->reputation_ += delta;
  this->reputation_ =
      std::clamp(this->reputation_, kReputationMin, kReputationMax);

  if (this->reputation_ != before) {
    SAL_LOGD("SalonContext: reputation %d -> %d (delta %d)", before,
             this->reputation_, delta);
  }
}

void SalonContext::advanceDay() {
  ++this->day_;
  this->adjustReputation(-kDailyReputationDecay);
  SAL_LOGI("SalonContext: day %u begins (money=%d, rep=%d)", this->day_,
           this->money_, this->reputation_);
}

} // namespace salshalon::features::salon
