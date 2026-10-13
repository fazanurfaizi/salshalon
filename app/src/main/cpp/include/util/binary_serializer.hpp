#pragma once

#include <cstdint>
#include <ios>
#include <istream>
#include <ostream>
#include <string>
#include <type_traits>

namespace salshalon::util {

// Writes native-endian bytes. All supported Android ABIs are little-endian,
// so files are portable across arm64, armeabi-v7a, and x86_64. A big-endian
// port would require byte-swapping here.
template <typename T> void write_binary(std::ostream &os, const T &value) {
  static_assert(std::is_trivially_copyable_v<T>,
                "write_binary requires a trivially copyable type");
  os.write(reinterpret_cast<const char *>(&value), sizeof(T));
}

template <typename T> bool read_binary(std::istream &is, T &value) {
  is.read(reinterpret_cast<char *>(&value), sizeof(T));
  return is.gcount() == static_cast<std::streamsize>(sizeof(T));
}

void write_string(std::ostream &os, const std::string &str);

std::string read_string(std::istream &is);

uint32_t crc32(const uint8_t *data, size_t len);

} // namespace salshalon::util
