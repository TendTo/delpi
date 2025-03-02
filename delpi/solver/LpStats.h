/**
 * @author c3054737
 * @copyright 2025 delpi
 * @licence BSD 3-Clause License
 * LpStats struct.
 */
#pragma once

#include <cstddef>
#include <iosfwd>

#include "delpi/util/Stats.h"

namespace delpi {

/** Collection of statistics for the LP solver.  */
struct LpStats {
  explicit LpStats(const bool enabled, const std::string& class_name = "LpSolver")
      : solver_stats{enabled, class_name, "Total time spent in Optimise", "Total # of iterations"},
        parser_stats{enabled, class_name, "Time spent in the parser"},
        refinements{0},
        precision{0} {}
  IterationStats solver_stats;  ///< Time spent in the solver and number of iterations
  Stats parser_stats;           ///< Time spent in the parser
  std::size_t refinements;      ///< Number of iterative refinements.  Only meaningful for iterative refinement solvers
  std::size_t precision;        ///< Number of bits of precision used in the last iteration.
                                ///< Only meaningful for precision boosting solvers
};

std::ostream& operator<<(std::ostream& os, const LpStats& stats);

}  // namespace delpi

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::LpStats);

#endif
