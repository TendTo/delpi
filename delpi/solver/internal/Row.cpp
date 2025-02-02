/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/solver/internal/Row.h"

#include <ostream>
#include <sstream>

namespace delpi::internal {

std::ostream& operator<<(std::ostream& os, const Row& row) {
  os << "Row{ ";
  bool print_plus = false;
  for (const auto& [idx, coeff] : row.addends) {
    os << (print_plus ? " + " : "");
    if (coeff != 1) os << coeff << " * ";
    os << "x" << idx;
    print_plus = true;
  }
  return os << " in [ " << (row.lb.has_value() ? (std::stringstream{} << row.lb.value()).str() : "-inf") << " , "
            << (row.ub.has_value() ? (std::stringstream{} << row.ub.value()).str() : "inf") << " ] }";
}

}  // namespace delpi::internal
