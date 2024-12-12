/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/parser/mps/SenseType.h"

#include <cctype>
#include <cstddef>
#include <iostream>

#include "delpi/util/error.h"

namespace delpi::mps {

SenseType ParseSense(const std::string &sense) {
  std::size_t pos = sense.find_first_not_of(' ');
  return ParseSense(sense[pos]);
}
SenseType ParseSense(const char sense[]) {
  while (*sense == ' ') ++sense;
  return ParseSense(*sense);
}
SenseType ParseSense(char sense) {
  sense = std::tolower(sense);
  switch (sense) {
    case 'l':
      return SenseType::L;
    case 'e':
      return SenseType::E;
    case 'g':
      return SenseType::G;
    case 'n':
      return SenseType::N;
    default:
      DELPI_UNREACHABLE();
  }
}

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

}  // namespace delpi::mps
