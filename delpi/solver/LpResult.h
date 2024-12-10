/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * LpResult enum.
 */
#pragma once

#include <iosfwd>

namespace delpi {

enum class LpResult {
  UNSOLVED,       ///< The solver has not yet been run.
  OPTIMAL,        ///< The problem is optimal.
  DELTA_OPTIMAL,  ///< The delta-relaxation of the problem is optimal.
  UNBOUNDED,      ///< The problem is unbounded
  INFEASIBLE,     ///< The problem is infeasible.
  ERROR,          ///< An error occurred.
};

/**
 * Relax the @p result of the theory solver (i.e. transform OPTIMAL to DELTA_OPTIMAL).
 *
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
