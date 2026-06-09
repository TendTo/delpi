/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 * @file
 */
#include "delpi/parser/BufferLineSource.h"

#include "delpi/util/error.h"

namespace delpi {
BufferLineSource::BufferLineSource(const char *data, const std::size_t length) : cur_{data}, end_{data + length} {
  DELPI_ASSERT(data != nullptr || length == 0, "Data pointer cannot be null with a positive length");
}

bool BufferLineSource::nextLine(std::string_view &out) {
  if (cur_ >= end_) return false;
  const char *start = cur_;
  while (cur_ < end_ && *cur_ != '\n') ++cur_;
  std::size_t len = static_cast<std::size_t>(cur_ - start);
  // Strip \r
  if (len > 0 && start[len - 1] == '\r') --len;
  if (cur_ < end_) ++cur_;  // skip '\n'
  out = {start, len};
  return true;
}

void BufferLineSource::initialize(const char *data, const std::size_t length) {
  cur_ = data;
  end_ = data + length;
}

}  // namespace delpi
