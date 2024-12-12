/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "LpResult.h"

#include <ostream>

#include "delpi/util/error.h"

namespace delpi {

std::ostream &operator<<(std::ostream &os, const LpResult result) {
  switch (result) {
    case LpResult::UNSOLVED:
      return os << "unsolved";
    case LpResult::OPTIMAL:
      return os << "optimal";
    case LpResult::DELTA_OPTIMAL:
      return os << "delta-optimal";
    case LpResult::UNBOUNDED:
      return os << "unbounded";
    case LpResult::INFEASIBLE:
      return os << "infeasible";
    case LpResult::ERROR:
      return os << "error";
    default:
      DELPI_UNREACHABLE();
  }
}

LpResult operator~(const LpResult result) {
  switch (result) {
    case LpResult::OPTIMAL:
    case LpResult::DELTA_OPTIMAL:
      return LpResult::DELTA_OPTIMAL;
    default:
      return result;
  }
}

bool IsFeasible(const LpResult result) {
  switch (result) {
    case LpResult::OPTIMAL:
    case LpResult::DELTA_OPTIMAL:
    case LpResult::UNBOUNDED:
      return true;
    default:
      return false;
  }
}
int ExitCode(const LpResult result) {
  switch (result) {
    case LpResult::OPTIMAL:
    case LpResult::DELTA_OPTIMAL:
    case LpResult::UNBOUNDED:
    case LpResult::INFEASIBLE:
      return 0;
    case LpResult::ERROR:
      return 1;
    default:
      return 2;
  }
}

}  // namespace delpi
