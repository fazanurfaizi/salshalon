#include "features/salon/SaveSystem.hpp"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <ios>
#include <istream>
#include <optional>

#include <sstream>
#include <string>
#include <vector>

#include "core/Log.hpp"
#include "util/binary_serializer.hpp"

namespace salshalon::features::salon {

using namespace salshalon::util;

namespace {
// Container layout:
//   [MAGIC_NUMBER : uint32]   identifies the file as a Salshalon save
//   [FORMAT_VER   : uint32]   container format version (not profile version)
//   [PAYLOAD_SIZE : uint64]   number of payload bytes
//   [CHECKSUM     : uint32]   CRC32 of payload
//   [PAYLOAD      : PAYLOAD_SIZE bytes]
constexpr uint32_t kMagicNumber = 0x53414C56u;
constexpr uint32_t kFormatVersion = 1u;

// Refuse anything larger than this (protects against corrupt sizes).
constexpr uint64_t kMaxPayloadSize = 64ull * 1024ull * 1024ull;

// Refuse implausibly large station arrays.
constexpr uint32_t kMaxStations = 1'000'000u;

void serializeProfile(std::ostream &os, const SalonProfile &p) {
  write_binary(os, p.version);
  write_binary(os, p.money);
  write_binary(os, p.reputation);
  write_binary(os, p.day);
  write_binary(os, p.totalCustomersServed);
  write_binary(os, p.bestDayCustomers);

  const uint32_t stationCount = static_cast<uint32_t>(p.stations.size());
  write_binary(os, stationCount);
  for (const auto &s : p.stations) {
    write_string(os, s.archetypeId);
    write_binary(os, static_cast<int32_t>(s.gridX));
    write_binary(os, static_cast<int32_t>(s.gridY));
  }

  write_string(os, p.floorId);
  write_string(os, p.wallId);

  const uint32_t unlockCount =
      static_cast<uint32_t>(p.unlockedCosmeticIds.size());
  write_binary(os, unlockCount);
  for (const auto &u : p.unlockedCosmeticIds) {
    write_string(os, u);
  }

  const uint32_t decorCount = static_cast<uint32_t>(p.decors.size());
  write_binary(os, decorCount);
  for (const auto &d : p.decors) {
    write_string(os, d.archetypeId);
    write_binary(os, static_cast<int32_t>(d.gridX));
    write_binary(os, static_cast<int32_t>(d.gridY));
  }

  write_binary(os, p.hasSeenTutorial);
}

std::optional<SalonProfile> deserializeProfile(std::istream &ifs) {
  SalonProfile p;

  if (!read_binary(ifs, p.version))
    return std::nullopt;
  if (p.version > SalonProfile::kCurrentVersion) {
    SAL_LOGE("SaveSystem: save version %u is newer than %u, refusing to load",
             p.version, SalonProfile::kCurrentVersion);
    return std::nullopt;
  }
  const uint32_t fileVersion = p.version;

  if (!read_binary(ifs, p.money) || !read_binary(ifs, p.reputation) ||
      !read_binary(ifs, p.day) || !read_binary(ifs, p.totalCustomersServed) ||
      !read_binary(ifs, p.bestDayCustomers)) {
    return std::nullopt;
  }

  uint32_t count = 0;
  if (!read_binary(ifs, count))
    return std::nullopt;
  if (count > kMaxStations) {
    SAL_LOGE("SaveSystem: station count %u is implausible", count);
    return std::nullopt;
  }

  p.stations.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    StationInstance s;
    s.archetypeId = read_string(ifs);
    int32_t gx = 0, gy = 0;
    if (!read_binary(ifs, gx) || !read_binary(ifs, gy))
      return std::nullopt;

    s.gridX = gx;
    s.gridY = gy;

    if (s.archetypeId.empty() || s.gridX < 0 || s.gridY < 0) {
      SAL_LOGW("SaveSystem: skipping malformed station entry");
      continue;
    }
    p.stations.push_back(std::move(s));
  }

  p.floorId = read_string(ifs);
  p.wallId = read_string(ifs);

  uint32_t unlockCount = 0;
  if (!read_binary(ifs, unlockCount))
    return std::nullopt;
  if (unlockCount > 1024u)
    return std::nullopt;
  p.unlockedCosmeticIds.reserve(unlockCount);
  for (uint32_t i = 0; i < unlockCount; ++i) {
    auto id = read_string(ifs);
    if (!id.empty())
      p.unlockedCosmeticIds.push_back(std::move(id));
  }

  uint32_t decorCount = 0;
  if (!read_binary(ifs, decorCount))
    return std::nullopt;
  if (decorCount > kMaxStations)
    return std::nullopt;
  p.decors.reserve(decorCount);
  for (uint32_t i = 0; i < decorCount; ++i) {
    DecorInstance d;
    d.archetypeId = read_string(ifs);
    int32_t gx = 0, gy = 0;
    if (!read_binary(ifs, gx) || !read_binary(ifs, gy))
      return std::nullopt;
    d.gridX = gx;
    d.gridY = gy;
    if (d.archetypeId.empty() || d.gridX < 0 || d.gridY < 0)
      continue;
    p.decors.push_back(std::move(d));
  }

  if (fileVersion >= 2) {
    bool seen = false;
    if (!read_binary(ifs, seen))
      return std::nullopt;
    p.hasSeenTutorial = seen;
  }

  if (!ifs)
    return std::nullopt;

  p.version = SalonProfile::kCurrentVersion;
  return p;
}

} // namespace

bool SaveSystem::save(const std::string &filePath,
                      const SalonProfile &profile) {
  const std::string tempPath = filePath + ".tmp";

  try {
    std::ostringstream payloadStream(std::ios::out | std::ios::binary);
    serializeProfile(payloadStream, profile);
    const std::string payload = payloadStream.str();

    const uint64_t payloadSize = static_cast<uint64_t>(payload.size());
    const uint32_t checksum = util::crc32(
        reinterpret_cast<const uint8_t *>(payload.data()), payload.size());

    std::ofstream out(tempPath, std::ios::trunc | std::ios::binary);
    if (!out.is_open()) {
      SAL_LOGE("SaveSystem: cannot open %s for writing", tempPath.c_str());
      return false;
    }

    util::write_binary(out, kMagicNumber);
    util::write_binary(out, kFormatVersion);
    util::write_binary(out, payloadSize);
    util::write_binary(out, checksum);
    out.write(payload.data(), static_cast<std::streamsize>(payload.size()));
    out.close();

    if (!out) {
      SAL_LOGE("SaveSystem: write failed on %s", tempPath.c_str());
      std::remove(tempPath.c_str());
      return false;
    }

    // Rename file
    if (std::rename(tempPath.c_str(), filePath.c_str()) != 0) {
      SAL_LOGE("SaveSystem: rename failed %s -> %s", tempPath.c_str(),
               filePath.c_str());
      std::remove(tempPath.c_str());
      return false;
    }
  } catch (const std::exception &e) {
    SAL_LOGE("SaveSystem: exception during save: %s", e.what());
    return false;
  }

  SAL_LOGI("SaveSystem: saved %zu stations, $%d, rep %d, day %u",
           profile.stations.size(), profile.money, profile.reputation,
           profile.day);
  return true;
}

std::optional<SalonProfile> SaveSystem::load(const std::string &filePath) {
  std::ifstream in(filePath, std::ios::binary);
  if (!in.is_open()) {
    SAL_LOGI("SaveSystem: no save file at %s", filePath.c_str());
    return std::nullopt;
  }

  try {
    uint32_t magic = 0;
    if (!util::read_binary(in, magic) || magic != kMagicNumber) {
      SAL_LOGE("SaveSystem: bad magic in %s", filePath.c_str());
      return std::nullopt;
    }

    uint32_t formatVersion = 0;
    if (!util::read_binary(in, formatVersion)) {
      SAL_LOGE("SaveSystem: truncated header in %s", filePath.c_str());
      return std::nullopt;
    }
    if (formatVersion > kFormatVersion) {
      SAL_LOGE("SaveSystem: container version %u newer than %u in %s",
               formatVersion, kFormatVersion, filePath.c_str());
      return std::nullopt;
    }

    uint64_t payloadSize = 0;
    if (!util::read_binary(in, payloadSize))
      return std::nullopt;
    if (payloadSize > kMaxPayloadSize) {
      SAL_LOGE("SaveSystem: payload size %llu too large in %s",
               static_cast<unsigned long long>(payloadSize), filePath.c_str());
      return std::nullopt;
    }

    uint32_t expectedChecksum = 0;
    if (!util::read_binary(in, expectedChecksum))
      return std::nullopt;

    std::vector<char> payload(static_cast<size_t>(payloadSize));
    if (payloadSize > 0) {
      in.read(payload.data(), static_cast<std::streamsize>(payloadSize));
      if (!in) {
        SAL_LOGE("SaveSystem: truncated payload in %s", filePath.c_str());
        return std::nullopt;
      }
    }

    const uint32_t actualChecksum = util::crc32(
        reinterpret_cast<const uint8_t *>(payload.data()), payload.size());
    if (actualChecksum != expectedChecksum) {
      SAL_LOGE("SaveSystem: checksum mismatch in %s (expected %08X, got %08X)",
               filePath.c_str(), expectedChecksum, actualChecksum);
      return std::nullopt;
    }

    std::istringstream payloadStream(
        std::string(payload.begin(), payload.end()), std::ios::binary);
    auto profile = deserializeProfile(payloadStream);
    if (profile) {
      SAL_LOGI("SaveSystem: loaded $%d, rep %d, day %u, %zu stations",
               profile->money, profile->reputation, profile->day,
               profile->stations.size());
    }
    return profile;
  } catch (const std::exception &e) {
    SAL_LOGE("SaveSystem: parse error in %s: %s", filePath.c_str(), e.what());
    return std::nullopt;
  }
}

std::string SaveSystem::profilePath(const std::string &internalDataDir) {
  if (internalDataDir.empty())
    return "save.bin";
  if (internalDataDir.back() == '/')
    return internalDataDir + "save.bin";
  return internalDataDir + "/save.bin";
}

} // namespace salshalon::features::salon
