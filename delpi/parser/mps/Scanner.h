/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 * Scanner class.
 */
#pragma once

#include "delpi/parser/mps/Driver.h"

namespace delpi::mps {

// Forward declaration
class BufferLineSource;

/**
 * NewMpsScanner takes care of parsing the MPS file line by line,
 * keeping an internal state and invoking the correct method of the MpsDriver.
 */
class NewMpsScanner {
 public:
  /**
   * Create a new scanner object.
   * @param driver driver that receives parsing events
   */
  explicit NewMpsScanner(MpsDriver& driver);

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

 private:
  /**
   * Parse all lines from a line-oriented source.
   * @param src line source
   * @return whether parsing succeeded
   */
  bool ParseLines(BufferLineSource& src);

  /**
   * Handle a comment line.
   * @param line line contents
   */
  void HandleComment(std::string_view line);

  /**
   * Handle the NAME section line.
   * @param line line contents
   */
  void HandleName(std::string_view line);

  /**
   * Handle the OBJSENSE section line.
   * @param line line contents
   */
  void HandleObjSense(std::string_view line);

  /**
   * Handle the OBJNAME section line.
   * @param line line contents
   */
  void HandleObjName(std::string_view line);

  /**
   * Handle a ROWS section line.
   * @param line line contents
   */
  void HandleRows(std::string_view line);

  /**
   * Handle a COLUMNS section line.
   * @param line line contents
   */
  void HandleColumns(std::string_view line);

  /**
   * Handle an RHS section line.
   * @param line line contents
   */
  void HandleRhs(std::string_view line);

  /**
   * Handle a RANGES section line.
   * @param line line contents
   */
  void HandleRanges(std::string_view line);

  /**
   * Handle a BOUNDS section line.
   * @param line line contents
   */
  void HandleBounds(std::string_view line);

  MpsDriver& driver_;    ///< Driver reference. Called upon parsing a line
  std::size_t line_no_;  ///< Current line number, used for error reporting
};

}  // namespace delpi::mps
