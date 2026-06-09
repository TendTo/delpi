/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Column struct.
 */
#pragma once

#include <iosfwd>
#include <optional>

#include "delpi/libs/gmp.h"
#include "delpi/symbolic/Variable.h"

namespace delpi::lp {

/**
 * Data structure representing a column in the LP solver as it gets parsed from an LP file.
 * Missing bounds are represented by `std::nullopt` and may have different meanings.
 * If any of the bounds is set, the variable is bounded from that direction.
 * A missing upper bound on a non-binary variable means that the variable is unbounded in the positive direction.
 * A missing upper bound on a binary variable means that the variable is bounded by 1.
 * A missing lower bound means that the variable is non-negative (@f$ x \geq 0 @f$).
 */
struct Column {
  Column() = default;
  explicit Column(const Variable& _var, const bool _is_binary = false)
      : var{_var}, lb{std::nullopt}, ub{std::nullopt}, is_binary{_is_binary} {}
  Column(const Variable& _var, const mpq_class& _ub) : var{_var}, lb{std::nullopt}, ub{_ub}, is_binary{false} {}
  Column(const Variable& _var, const mpq_class& _lb, const mpq_class& _ub)
      : var{_var}, lb{_lb}, ub{_ub}, is_binary{false} {}
  Variable var;                 ///< Variable.
  std::optional<mpq_class> lb;  ///< Lower bound.
  std::optional<mpq_class> ub;  ///< Upper bound.
  bool is_binary{false};        ///< Indicates if the variable is binary.
};

std::ostream& operator<<(std::ostream& os, const Column& column);

}  // namespace delpi::lp

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::lp::Column)

#endif
