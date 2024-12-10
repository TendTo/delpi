/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence Apache-2.0 license
 * FormulaKind enum.
 */
#pragma once

#include <iosfwd>

namespace delpi {

/** Kinds of symbolic formulas. */
enum class FormulaKind {
  Eq,   ///< =
  Neq,  ///< !=
  Gt,   ///< >
  Geq,  ///< >=
  Lt,   ///< <
  Leq,  ///< <=
};

/**
 * FormulaKind that continues to match the satisfaction of the formula after both sides have been multiplied by -1.
 * @param kind original formula kind
 * @return negated kind
 */
FormulaKind operator-(FormulaKind kind);
/**
 * FormulaKind that inverts the satisfaction of the formula.
 * @param kind original formula kind
 * @return inverted kind
 */
FormulaKind operator!(FormulaKind kind);

std::ostream& operator<<(std::ostream& os, FormulaKind kind);

}  // namespace delpi

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::FormulaKind);

#endif
