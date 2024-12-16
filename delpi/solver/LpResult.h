/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * LpResult enum.
 */
#pragma once

#include <iosfwd>

namespace delpi {

/** Possible outcomes of the LP solver. */
enum class LpResult {
  UNSOLVED,       ///< The solver has not yet been run.
  OPTIMAL,        ///< The problem is optimal.
  DELTA_OPTIMAL,  ///< The delta-relaxation of the problem is optimal.
  UNBOUNDED,      ///< The problem is unbounded
  INFEASIBLE,     ///< The problem is infeasible.
  ERROR,          ///< An error occurred.
};

/**
 * Check if the `result` obtained by the LpSolver implies that the problem is feasible.
 * @param result result to check
 * @return true if the result implies that the problem is feasible
 * @return false if the result implies that the problem is infeasible
 */
bool IsFeasible(LpResult result);

/**
 * Convert the result in
 * @param result result used to produce the exit code
 * @return 0 if the problem is optimal, delta-optimal, unbounded or infeasible
 * @return 1 if an error was detected
 * @return 2 any other case
 */
int ExitCode(LpResult result);

/**
 * Relax the `result` of the theory solver (i.e. transform OPTIMAL to DELTA_OPTIMAL).
 * All other results are left unchanged.
 * @param result result to relax
 * @return relaxed result
 */
LpResult operator~(LpResult result);
std::ostream &operator<<(std::ostream &os, LpResult result);

}  // namespace delpi

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::LpResult)

#endif
