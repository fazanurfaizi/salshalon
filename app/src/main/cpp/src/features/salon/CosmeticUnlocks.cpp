#include "features/salon/CosmeticUnlocks.hpp"

#include "core/Log.hpp"
#include "features/salon/CosmeticCatalog.hpp"
#include "features/salon/SalonContext.hpp"

namespace salshalon::features::salon::CosmeticUnlocks {

namespace {

bool qualified(const UnlockCondition &condition, const SalonContext &ctx) {
  using T = UnlockCondition::Type;
  switch (condition.type) {
  case T::Always:
    return true;
  case T::TotalServed:
    return ctx.totalCustomersServed() >=
           static_cast<uint32_t>(condition.threshold);
  case T::Reputation:
    return ctx.reputation() >= condition.threshold;
  case T::BestDay:
    return ctx.bestDayCustomers() >= static_cast<uint32_t>(condition.threshold);
  }
  return false;
}

} // namespace

std::vector<std::string> checkAndApply(SalonContext &ctx) {
  std::vector<std::string> newlyUnlocked;

  for (const auto &c : CosmeticCatalog::all()) {
    if (ctx.isCosmeticUnlocked(c.id))
      continue;
    if (!qualified(c.unlock, ctx))
      continue;

    if (ctx.unlockCosmetic(c.id)) {
      newlyUnlocked.push_back(c.displayName);
      SAL_LOGI("Unlock: %s (%s)", c.displayName.c_str(), c.id.c_str());
    }
  }

  return newlyUnlocked;
}

} // namespace salshalon::features::salon::CosmeticUnlocks
