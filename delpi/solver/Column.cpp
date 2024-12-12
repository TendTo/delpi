/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "Column.h"

#include <ostream>

namespace delpi {

std::ostream& operator<<(std::ostream& os, const Column& column) {
  os << "Column{ " << column.var << " in [ " << column.lb.value_or(mpq_class{0}) << " , ";
  if (column.ub.has_value()) {
    os << column.ub.value();
  } else {
    os << "inf";
  }
  return os << " ] , obj=" << column.obj.value_or(mpq_class{0}) << " }";
}

}  // namespace delpi
