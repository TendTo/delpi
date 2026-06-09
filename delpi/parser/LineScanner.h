/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 * @file
 * LineScanner class.
 */
#pragma once

#include <string>
#include <string_view>

namespace delpi {

// Forward declaration
class BufferLineSource;

/**
 * LineScanner takes care of parsing the MPS file line by line,
 * keeping an internal state and invoking the correct method of the MpsDriver.
 */
class LineScanner {
 public:
  LineScanner() : filename_{}, line_no_{0} {}
  virtual ~LineScanner() = default;

  /**
   * Parse an MPS file from disk.
   * @param filename path to the MPS file
   * @return whether parsing succeeded
   */
  bool ParseFile(const std::string& filename);

  /**
   * Parse an MPS file from an in-memory string.
   * @param input MPS contents
   * @return whether parsing succeeded
   */
  bool ParseString(std::string_view input);

 protected:
  /**
   * Parse all lines from a line-oriented source.
   * @param src line source
   * @return whether parsing succeeded
   */
  virtual bool ParseLines(BufferLineSource& src) = 0;

  std::string filename_;  ///< Name of the file being parsed, used for error reporting
  std::size_t line_no_;   ///< Current line number, used for error reporting
};

}  // namespace delpi
