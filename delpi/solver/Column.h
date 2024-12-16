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

namespace delpi {

/**
 * Convenient structure representing a column in the LP solver.
 * A missing lower bound means that the variable is unbounded in the negative direction.
 * A missing upper bound means that the variable is unbounded in the positive direction.
 * A missing objective function coefficient means that the variable does not participate in the objective function.
 */
struct Column {
  Variable var;                  ///< Variable.
  std::optional<mpq_class> lb;   ///< Lower bound.
  std::optional<mpq_class> ub;   ///< Upper bound.
  std::optional<mpq_class> obj;  ///< Objective function coefficient.
};

std::ostream& operator<<(std::ostream& os, const Column& column);

}  // namespace delpi

#ifndef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::Column)

#endif
