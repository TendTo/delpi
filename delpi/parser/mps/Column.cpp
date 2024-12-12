/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "Column.h"

#include <ostream>
#include <sstream>

namespace delpi::mps {

std::ostream& operator<<(std::ostream& os, const Column& column) {
  return os << "Column{ " << column.var << " in [ "
            << (column.is_infinite_lb ? (std::stringstream{} << column.lb.value_or(mpq_class{0})).str() : "-inf")
            << " , " << (column.ub.has_value() ? (std::stringstream{} << column.ub.value()).str() : "inf") << " ] }";
}

}  // namespace delpi::mps
