/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * LinearMonomial struct.
 */
#pragma once

#include "delpi/libs/gmp.h"
#include "delpi/symbolic/Variable.h"

namespace delpi {

/** Simple structure representing a variable with an associated coefficient */
struct LinearMonomial {
  Variable var;     ///< Variable
  mpq_class coeff;  ///< Rational coefficient
};

std::ostream& operator<<(std::ostream&, const LinearMonomial& linear_monomial);

}  // namespace delpi

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::LinearMonomial);

#endif
