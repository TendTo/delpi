/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 * @file
 * MappedFileSource class.
 */
#pragma once

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "delpi/parser/BufferLineSource.h"

namespace delpi {

/**
 * A memory-mapped file source for efficiently reading files line by line.
 * MappedFileSource uses memory-mapped file techniques to handle files,
 * allowing for high-performance sequential file reading.
 * @note The file is mapped in read-only mode, ant it is freed once the MappedFileSource is destroyed.
 */
class MappedFileSource : public BufferLineSource {
 public:
  /**
   * Construct a MappedFileSource for the given file.
   * @param filename The path to the file to be memory-mapped and read.
   * @throws boost::interprocess::interprocess_exception if the file cannot be mapped.
   */
  explicit MappedFileSource(const char *const filename);

 private:
  const boost::interprocess::file_mapping
      file_mapping_;  ///< The file mapping object that manages the mapping of the file into memory.
  boost::interprocess::mapped_region
      region_;  ///< The mapped region object that provides access to the mapped file content.
};

}  // namespace delpi
