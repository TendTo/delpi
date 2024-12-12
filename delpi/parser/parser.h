/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Parser class.
 */
#pragma once

#include <memory>

#include "delpi/parser/Driver.h"
#include "delpi/solver/LpSolver.h"
#include "delpi/util/Stats.h"

namespace delpi {

/**
 * Facade class that hides the implementation details of the parser.
 * Parsers usually utilise a driver, a parser and a scanner to go through the input file and populate the LpSolver.
 * The correct driver is selected based on the actual format set in the Config.
 */
class Parser {
 public:
  /**
   * Construct a new Parser object to populate the given `lp_solver`.
   * @param lp_solver LP solver to populate
   */
  explicit Parser(LpSolver& lp_solver);

  /**
   * Get the correct driver instance based on the actual format set in the Config.
   * @param lp_solver LP solver to populate
   * @return A new instance of the correct driver based on the actual format set in the Config
   */
  static std::unique_ptr<Driver> GetDriverInstance(LpSolver& lp_solver);

  /** @getter{stats, driver} */
  [[nodiscard]] const Stats& stats() const;
  /** @getter{driver, driver} */
  [[nodiscard]] const std::unique_ptr<Driver>& driver() const { return driver_; }

 private:
  std::unique_ptr<Driver> driver_;  ///< Driver to use based on the input file format
};

}  // namespace delpi
