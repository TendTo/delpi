/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "LinearMonomial.h"

#include <ostream>

namespace delpi {

std::ostream& operator<<(std::ostream& os, const LinearMonomial& linear_monomial) {
  return os << "( " << linear_monomial.coeff << " * " << linear_monomial.var << " )";
}

}  // namespace delpi