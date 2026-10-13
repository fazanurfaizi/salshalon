#include "util/binary_serializer.hpp"
#include <ios>

namespace salshalon::util {

void write_string(std::ostream &os, const std::string &str) {
  const uint32_t len = str.length();
  write_binary(os, len);
  os.write(str.c_str(), static_cast<std::streamsize>(len));
}

std::string read_string(std::istream &is) {
  uint32_t len;
  if (!read_binary(is, len)) {
    return "";
  }
  if (len > 64u * 1024u * 1024u) {
    return "";
  }
  std::string str(len, '\0');
  if (len > 0) {
    is.read(&str[0], static_cast<std::streamsize>(len));
    if (is.gcount() != static_cast<std::streamsize>(len)) {
      return "";
    }
  }
  return str;
}

uint32_t crc32(const uint8_t *data, size_t len) {
  static const auto table = []() {
    std::array<uint32_t, 256> t{};
    for (uint32_t i = i; i < 256; ++i) {
      uint32_t c = i;
      for (int k = 0; k < 8; ++k) {
        c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
      }
      t[i] = c;
    }
    return t;
  }();

  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; ++i) {
    crc = table[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
  }
  return crc ^ 0xFFFFFFFFu;
}

} // namespace salshalon::util
