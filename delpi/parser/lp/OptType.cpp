/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/parser/lp/OptType.h"

#include <ostream>
#include <string>

#include "delpi/util/error.h"
#include "delpi/util/strings.hpp"

namespace delpi::lp {

std::ostream &operator<<(std::ostream &os, const OptType &sense) {
  switch (sense) {
    case OptType::MAX:
      return os << "MAX";
    case OptType::MIN:
      return os << "MIX";
    default:
      DELPI_UNREACHABLE();
  }
}

}  // namespace delpi::lp
