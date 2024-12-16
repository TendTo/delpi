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

namespace delpi {

/**
 * Get the correct driver instance based on the actual format set in the Config.
 * @param lp_solver LP solver to populate
 * @return A new instance of the correct driver based on the actual format set in the Config
 */
std::unique_ptr<Driver> GetDriverInstance(LpSolver& lp_solver);

}  // namespace delpi
