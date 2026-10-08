#include "core/AssetManager.hpp"

#include <android/asset_manager.h>

#include "core/Log.hpp"

namespace salshalon::core {

bool AssetManager::exists(const char *path) const {
  if (this->manager_ == nullptr || path == nullptr)
    return false;
  AAsset *asset = AAssetManager_open(this->manager_, path, AASSET_MODE_UNKNOWN);
  if (asset == nullptr)
    return false;
  AAsset_close(asset);
  return true;
}

std::vector<uint8_t> AssetManager::read(const char *path) const {
  std::vector<uint8_t> data;
  if (this->manager_ == nullptr || path == nullptr)
    return data;

  AAsset *asset = AAssetManager_open(this->manager_, path, AASSET_MODE_BUFFER);
  if (asset == nullptr) {
    SAL_LOGW("AssetManager: asset not found: %s", path);
    return data;
  }

  const off_t length = AAsset_getLength(asset);
  if (length > 0) {
    data.resize(static_cast<size_t>(length));
    const int bytesRead = AAsset_read(asset, data.data(), data.size());
    if (bytesRead != static_cast<int>(data.size())) {
      SAL_LOGE("AssetManager: short reads on %s (%d / %zu)", path, bytesRead,
               data.size());
      data.clear();
    }
  }
  AAsset_close(asset);
  return data;
}

std::string AssetManager::readText(const char *path) const {
  std::vector<uint8_t> bytes = this->read(path);
  return std::string(bytes.begin(), bytes.end());
}

} // namespace salshalon::core
