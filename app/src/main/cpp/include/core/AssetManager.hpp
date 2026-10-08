#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct AAssetManager;

namespace salshalon::core {

class AssetManager {
public:
  AssetManager() = default;
  explicit AssetManager(AAssetManager *manager) : manager_(manager) {}

  void setAssetManager(AAssetManager *manager) { this->manager_ = manager; }
  AAssetManager *native() const { return this->manager_; }

  bool exists(const char *path) const;

  /// Reads the whole asset into memory. Returns an empty vector on failure.
  std::vector<uint8_t> read(const char *path) const;

  /// Reads a text asset (shaders, json, ...). Returns "" on failure.
  std::string readText(const char *path) const;

private:
  AAssetManager *manager_ = nullptr;
};

} // namespace salshalon::core
