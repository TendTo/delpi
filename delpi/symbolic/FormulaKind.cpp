/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence Apache-2.0 license
 */
#include "delpi/symbolic/FormulaKind.h"

#include <ostream>

#include "delpi/util/error.h"

namespace delpi {

FormulaKind operator-(const FormulaKind kind) {
  switch (kind) {
    case FormulaKind::Gt:
      return FormulaKind::Lt;
    case FormulaKind::Geq:
      return FormulaKind::Leq;
    case FormulaKind::Lt:
      return FormulaKind::Gt;
    case FormulaKind::Leq:
      return FormulaKind::Geq;
    default:
      return kind;
  }
}
FormulaKind operator!(const FormulaKind kind) {
  switch (kind) {
    case FormulaKind::Eq:
      return FormulaKind::Neq;
    case FormulaKind::Neq:
      return FormulaKind::Eq;
    case FormulaKind::Gt:
      return FormulaKind::Leq;
    case FormulaKind::Geq:
      return FormulaKind::Lt;
    case FormulaKind::Lt:
      return FormulaKind::Geq;
    case FormulaKind::Leq:
      return FormulaKind::Gt;
    default:
      DELPI_UNREACHABLE();
  }
}
std::ostream& operator<<(std::ostream& os, const FormulaKind kind) {
  switch (kind) {
    case FormulaKind::Eq:
      return os << "=";
    case FormulaKind::Neq:
      return os << "!=";
    case FormulaKind::Gt:
      return os << ">";
    case FormulaKind::Geq:
      return os << ">=";
    case FormulaKind::Lt:
      return os << "<";
    case FormulaKind::Leq:
      return os << "<=";
    default:
      DELPI_UNREACHABLE();
  }
}

}  // namespace delpi
