/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 * @file
 * LineScanner class.
 */
#include "delpi/parser/LineScanner.h"

#include "delpi/parser/MappedFileSource.h"
#include "delpi/util/logging.h"

namespace delpi {

bool LineScanner::ParseFile(const std::string &filename) {
  try {
    filename_ = filename;
    MappedFileSource src(filename.c_str());
    ParseLines(src);
  } catch (const boost::interprocess::interprocess_exception &e) {
    DELPI_ERROR_FMT("Error reading file '{}': {}", filename, e.what());
    return false;
  }
  return true;
}

bool LineScanner::ParseString(const std::string_view input) {
  BufferLineSource src{input};
  ParseLines(src);
  return true;
}

}  // namespace delpi
