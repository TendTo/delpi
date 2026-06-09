/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 * @file
 */
#include "delpi/parser/MappedFileSource.h"

namespace delpi {

MappedFileSource::MappedFileSource(const char* const filename)
    : BufferLineSource{nullptr, 0},
      file_mapping_{filename, boost::interprocess::read_only},
      region_{file_mapping_, boost::interprocess::read_only} {
  region_.advise(boost::interprocess::mapped_region::advice_sequential);
  initialize(static_cast<const char*>(region_.get_address()), region_.get_size());
}

}  // namespace delpi
