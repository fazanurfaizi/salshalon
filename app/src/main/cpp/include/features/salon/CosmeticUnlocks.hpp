#pragma once

#include <string>
#include <vector>

namespace salshalon::features::salon {

class SalonContext;

namespace CosmeticUnlocks {

/// Iterates the catalog, checks each cosmetic's unlock condition against the
/// current profile stats, and unlocks any newly-qualifying items.
std::vector<std::string> checkAndApply(SalonContext &ctx);

} // namespace CosmeticUnlocks

} // namespace salshalon::features::salon
