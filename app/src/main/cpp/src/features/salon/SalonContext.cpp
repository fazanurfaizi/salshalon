#include "features/salon/SalonContext.hpp"

#include <algorithm>

#include "core/Log.hpp"
#include "features/salon/cosmetics/CosmeticCatalog.hpp"

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
  SAL_LOGI("SalonContext: day %d begins (money=%d, rep=%d)", this->day_,
           this->money_, this->reputation_);
}

void SalonContext::onCustomerServed(int32_t payout) {
  this->earn(payout);
  this->adjustReputation(1);
  ++this->totalCustomersServed_;
}

void SalonContext::onCustomerLost() { this->adjustReputation(-5); }

void SalonContext::recordDayResult(uint32_t served) {
  if (served > this->bestDayCustomers_) {
    this->bestDayCustomers_ = served;
    SAL_LOGI("SalonContext: new best day — %u customers", served);
  }
}

bool SalonContext::isCosmeticUnlocked(const std::string &id) const {
  if (const auto *arch = CosmeticCatalog::find(id)) {
    if (arch->unlock.type == UnlockCondition::Type::Always)
      return true;
  }
  for (const auto &u : this->unlockedCosmeticIds_) {
    if (u == id)
      return true;
  }
  return false;
}

bool SalonContext::unlockCosmetic(const std::string &id) {
  if (isCosmeticUnlocked(id))
    return false;
  this->unlockedCosmeticIds_.push_back(id);
  return true;
}

void SalonContext::setCurrentFloor(const std::string &id) {
  if (!isCosmeticUnlocked(id))
    return;
  const auto *arch = CosmeticCatalog::find(id);
  if (arch == nullptr || arch->kind != CosmeticKind::Floor)
    return;
  this->floorId_ = id;
}

void SalonContext::setCurrentWall(const std::string &id) {
  if (!isCosmeticUnlocked(id))
    return;
  const auto *arch = CosmeticCatalog::find(id);
  if (arch == nullptr || arch->kind != CosmeticKind::Wall)
    return;
  this->wallId_ = id;
}

SalonProfile SalonContext::toProfile() const {
  SalonProfile p;
  p.version = SalonProfile::kCurrentVersion;
  p.money = money_;
  p.reputation = reputation_;
  p.day = static_cast<uint32_t>(day_);
  p.totalCustomersServed = totalCustomersServed_;
  p.bestDayCustomers = bestDayCustomers_;
  p.stations = layout_.stations();
  p.decors = layout_.decors();
  p.floorId = floorId_;
  p.wallId = wallId_;
  p.unlockedCosmeticIds = unlockedCosmeticIds_;
  p.hasSeenTutorial = tutorialSeen_;
  return p;
}

void SalonContext::applyProfile(const SalonProfile &profile) {
  money_ = profile.money;
  reputation_ = profile.reputation;
  day_ = static_cast<int32_t>(profile.day);
  totalCustomersServed_ = profile.totalCustomersServed;
  bestDayCustomers_ = profile.bestDayCustomers;

  floorId_ = profile.floorId.empty() ? "floor_wood" : profile.floorId;
  wallId_ = profile.wallId.empty() ? "wall_cream" : profile.wallId;
  unlockedCosmeticIds_ = profile.unlockedCosmeticIds;
  tutorialSeen_ = profile.hasSeenTutorial;

  layout_.reset();
  while (!layout_.stations().empty()) {
    const auto &s = layout_.stations().front();
    layout_.removeStationAt(s.gridX, s.gridY);
  }
  for (const auto &s : profile.stations) {
    layout_.placeStation(s);
  }
  for (const auto &d : profile.decors) {
    layout_.placeDecor(d);
  }

  SAL_LOGI("SalonContext: profile applied — $%d, rep %d, day %d, "
           "%zu stations, %zu decors, floor=%s wall=%s tutorialSeen=%d",
           money_, reputation_, day_, layout_.stations().size(),
           layout_.decors().size(), floorId_.c_str(), wallId_.c_str(),
           tutorialSeen_ ? 1 : 0);
}

} // namespace salshalon::features::salon
