#include "features/salon/BuildSystem.hpp"

#include "core/Log.hpp"
#include "features/salon/SalonContext.hpp"
#include "features/salon/stations/StationCatalog.hpp"

namespace salshalon::features::salon::BuildSystem {

Result place(SalonContext &ctx, const std::string &archetypeId, int col,
             int row) {
  const StationArchetype *arch = StationCatalog::find(archetypeId);
  if (arch == nullptr)
    return Result::NoSuchArchetype;

  SalonLayout &layout = ctx.layout();
  if (!layout.canPlaceStation(col, row))
    return Result::InvalidCell;
  if (ctx.money() < arch->cost)
    return Result::NotEnoughMoney;

  if (!ctx.trySpend(arch->cost))
    return Result::NotEnoughMoney;

  StationInstance inst{
      .archetypeId = archetypeId,
      .gridX = col,
      .gridY = row,
  };

  if (!layout.placeStation(inst)) {
    // Shouldn't happen - canPlaceStation was checked. Refund to be safe.
    ctx.earn(arch->cost);
    return Result::InvalidCell;
  }

  SAL_LOGI("Build: placed %s at (%d,%d) for $%d", archetypeId.c_str(), col, row,
           arch->cost);
  return Result::Success;
}

Result remove(SalonContext &ctx, int col, int row) {
  SalonLayout &layout = ctx.layout();

  const auto removed = layout.removeStationAt(col, row);
  if (!removed.has_value())
    return Result::NothingToRemove;

  const StationArchetype *arch = StationCatalog::find(removed->archetypeId);
  if (arch == nullptr)
    return Result::Success; // refunded nothing

  const int32_t refund = arch->cost / 2;
  ctx.earn(refund);

  SAL_LOGI("Build: removed %s for $%d refund", removed->archetypeId.c_str(),
           refund);
  return Result::Success;
}

const char *resultMessage(Result r) {
  switch (r) {
  case Result::Success:
    return "Done";
  case Result::NoSuchArchetype:
    return "Unknown item";
  case Result::InvalidCell:
    return "Can't place here";
  case Result::NotEnoughMoney:
    return "Not enough money";
  case Result::NothingToRemove:
    return "Nothing to remove";
  }
  return "Error";
}

} // namespace salshalon::features::salon::BuildSystem
