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
