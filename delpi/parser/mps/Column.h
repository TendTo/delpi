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

#ifndef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::Column)

#endif
