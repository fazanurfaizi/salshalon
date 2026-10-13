#pragma once

#include <optional>
#include <string>

#include "features/salon/SalonProfile.hpp"

namespace salshalon::features::salon {

/// Reads and writes SalonProfile instances to disk as a checksummed binary
/// container. All failures are non-fatal: a corrupt or missing file returns
/// nullopt, and the caller falls back to a fresh profile. A save failure
/// logs and returns false, but the game continues - losing a save is bad,
/// crashing is worse.
class SaveSystem {
public:
  /// Writes `profile` to `filePath` as JSON. Returns true on success.
  /// Creates the file if it does not exist; overwrites if it does.
  static bool save(const std::string &filePath, const SalonProfile &profile);

  /// Reads a profile from `filePath`. Returns nullopt if the file is missing,
  /// corrupt, or contains a version newer than this build understands.
  static std::optional<SalonProfile> load(const std::string &filePath);

  /// Convenience: the canonical path given the app's internal data dir.
  static std::string profilePath(const std::string &internalDataDir);
};

} // namespace salshalon::features::salon
