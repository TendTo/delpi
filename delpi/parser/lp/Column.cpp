/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/parser/lp/Column.h"

#include <ostream>
#include <sstream>

namespace delpi::lp {

std::ostream& operator<<(std::ostream& os, const Column& column) {
  return os << "Column{ " << column.var << " in [ " << column.lb.value_or(mpq_class{0}) << " , "
            << (column.is_binary ? (std::stringstream{} << column.ub.value_or(mpq_class{1})).str() : "inf") << " ] }";
}

}  // namespace delpi::lp
