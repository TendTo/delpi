/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/parser/mps/Sense.h"

#include <cctype>
#include <cstddef>
#include <iostream>

#include "delpi/util/error.h"

namespace delpi::mps {

Sense ParseSense(const std::string &sense) {
  std::size_t pos = sense.find_first_not_of(' ');
  return ParseSense(sense[pos]);
}
Sense ParseSense(const char sense[]) {
  while (*sense == ' ') ++sense;
  return ParseSense(*sense);
}
Sense ParseSense(char sense) {
  sense = std::tolower(sense);
  switch (sense) {
    case 'l':
      return Sense::L;
    case 'e':
      return Sense::E;
    case 'g':
      return Sense::G;
    case 'n':
      return Sense::N;
    default:
      DELPI_UNREACHABLE();
  }
}

std::ostream &operator<<(std::ostream &os, const Sense &sense) {
  switch (sense) {
    case Sense::L:
      return os << "L";
    case Sense::E:
      return os << "E";
    case Sense::G:
      return os << "G";
    case Sense::N:
      return os << "N";
    default:
      DELPI_UNREACHABLE();
  }
}

}  // namespace delpi::mps
