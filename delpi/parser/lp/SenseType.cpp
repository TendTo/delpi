/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/parser/lp/SenseType.h"

#include <ostream>
#include <string>

#include "delpi/util/error.h"

namespace delpi::lp {

std::ostream &operator<<(std::ostream &os, const SenseType &sense) {
  switch (sense) {
    case SenseType::L:
      return os << "L";
    case SenseType::E:
      return os << "E";
    case SenseType::G:
      return os << "G";
    case SenseType::N:
      return os << "N";
    default:
      DELPI_UNREACHABLE();
  }
}

}  // namespace delpi::lp
