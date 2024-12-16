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

namespace delpi::mps {

/**
 * Data structure representing a column in the LP solver as it gets parsed from an MPS file.
 * Missing bounds are represented by `std::nullopt` and may have different menings.
 * If any of the bounds is set, the variable is bounded from that direction.
 * A missing upper bound means that the variable is unbounded in the positive direction.
 * A missing lower bound with @ref is_infinite_lb = false means that the variable is non-negative (@f$ x \geq 0 @f$).
 * On the other had, if @ref is_infinite_lb = true, the variable is unbounded in the negative direction.
 */
struct Column {
  Column() = default;
  explicit Column(const Variable& _var) : var{_var}, lb{std::nullopt}, ub{std::nullopt}, is_infinite_lb{false} {}
  Column(const Variable& _var, const mpq_class& _lb, const mpq_class& _ub)
      : var{_var}, lb{_lb}, ub{_ub}, is_infinite_lb{false} {}
  Variable var;                 ///< Variable.
  std::optional<mpq_class> lb;  ///< Lower bound.
  std::optional<mpq_class> ub;  ///< Upper bound.
  bool is_infinite_lb{false};   ///< Indicates if the lower bound is negative infinity.
};

std::ostream& operator<<(std::ostream& os, const Column& column);

}  // namespace delpi::mps

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::mps::Column)

#endif
