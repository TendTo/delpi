/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 dlinear
 * @licence BSD 3-Clause License
 * Entry point of delpi.
 * Use the `-h` flag to show the help tooltip.
 */
#include <iostream>

#include "delpi.h"

int main(const int argc, const char* argv[]) {
  // Initialize the command line parser.
  delpi::ArgParser parser{};
  // Parse the command line arguments.
  parser.Parse(argc, argv);
  // Get the configuration from the command line arguments.
  const delpi::Config config = parser.ToConfig();

  // Setup the infinity values.
  const auto lp_solver{delpi::LpSolver::GetInstance(config)};
  if (!lp_solver->Parse()) {
    std::cerr << "Error parsing the input" << std::endl;
    return EXIT_FAILURE;
  }

  // Run the solver
  mpq_class precision{config.precision()};
  const delpi::LpResult result = lp_solver->Solve(precision);
  if (!config.silent() && lp_solver->ConflictingExpected(result)) {
    std::cerr << "WARNING: Expected " << lp_solver->expected() << " but got " << result << std::endl;
  }
  if (!config.silent() && config.verify() && IsFeasible(result)) {
    if (lp_solver->Verify())
      std::cout << "Model correctly satisfies the input" << std::endl;
    else
      std::cerr << "WARNING: Model does not satisfy the input" << std::endl;
  }
  if (!config.silent() && config.produce_models()) {
    fmt::println("Model: {}", lp_solver->model());
  }

  return ExitCode(result);
}
