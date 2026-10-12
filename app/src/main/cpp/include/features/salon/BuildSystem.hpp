#pragma once

#include <string>

namespace salshalon::features::salon {

class SalonContext;

/// Stateless logic for placing and removing stations. All state lives in
/// SalonContext (money and layout). BuildSystem just orchestrates.
namespace BuildSystem {

enum class Result {
  Success,
  NoSuchArchetype,
  InvalidCell,
  NotEnoughMoney,
  NothingToRemove,
};

/// Attempts to buy and place a station. Deducts money on success.
Result place(SalonContext &cxt, const std::string &archetypeId, int col,
             int row);

/// Removes the station at (col, row) and refunds 50% of its cost.
Result remove(SalonContext &ctx, int col, int row);

const char *resultMessage(Result r);

} // namespace BuildSystem

} // namespace salshalon::features::salon
