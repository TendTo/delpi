/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 * Scanner class.
 */
#pragma once

#include "delpi/parser/LineScanner.h"
#include "delpi/parser/mps/Driver.h"

namespace delpi {

// Forward declaration
class BufferLineSource;

namespace mps {
/**
 * MpsScanner takes care of parsing the MPS file line by line,
 * keeping an internal state and invoking the correct method of the MpsDriver.
 */
class MpsScanner : public LineScanner {
 public:
  /**
   * Create a new scanner object.
   * @param driver driver that receives parsing events
   */
  explicit MpsScanner(MpsDriver& driver);

 private:
  /**
   * Parse all lines from a line-oriented source.
   * @param src line source
   * @return whether parsing succeeded
   */
  bool ParseLines(BufferLineSource& src) override;

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
};

}  // namespace mps

}  // namespace delpi
