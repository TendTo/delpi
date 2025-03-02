/**
 * @author c3054737
 * @copyright 2025 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/solver/LpStats.h"

#include <ostream>

namespace delpi {

std::ostream& operator<<(std::ostream& os, const LpStats& stats) {
  os << "LpStats:\n"
     << "Solver stats: " << stats.solver_stats << "\n"
     << "Parser stats: " << stats.parser_stats << "\n"
     << "Refinements: " << stats.refinements << "\n"
     << "Precision: " << stats.precision;
  return os;
}

}  // namespace delpi
