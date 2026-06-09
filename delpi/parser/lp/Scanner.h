/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 * @file
 * Scanner class.
 */
#pragma once

#include <optional>
#include <string_view>

#include "delpi/libs/gmp.h"
#include "delpi/parser/BufferLineSource.h"
#include "delpi/parser/LineScanner.h"
#include "delpi/parser/lp/Driver.h"

namespace delpi::lp {

class LpScanner : public LineScanner {
 public:
  explicit LpScanner(LpDriver &driver) : driver_{driver} {}

  bool ParseLines(BufferLineSource &src) override;

 private:
  struct Term {
    std::optional<double> sign;
    std::optional<std::string_view> var;
    std::optional<mpq_class> coeff;
  };

  bool HandleComment(std::string_view line);

  bool HandleObjective(std::string_view line);

  bool HandleConstrains(std::string_view line);

  bool HandleBounds(std::string_view line);

  void HandleBinary(std::string_view line);

  void HandleGeneral(std::string_view line);

  /**
   * Given a word (i.e., a token without whitespaces, try parting a term by applyting the following regex:
   * `^(?<sign>[+-])?(?<coeff>[0-9]*(?:\.[0-9]?)?(?:[eE][-+]?[0-9]+)?|\d+\/\d+)?(?<var>[^0-9\/eE\.\-+].*)?$`,
   * @param token string without any whitespaces
   * @param prev accumulator for the current term
   * @return true if the accumulator has been updated successfully
   * @return false if an error has occurred
   */
  bool ParseTerm(std::string_view token, Term &prev);

  void ResetExpression();

  Term term_{};
  std::unordered_map<std::string_view, mpq_class> expression_;
  std::string_view identifier_;
  LpDriver &driver_;
};

}  // namespace delpi::lp