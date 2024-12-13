/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 dlinear
 * @licence BSD 3-Clause License
 * Entry point of delpi.
 * Use the `-h` flag to show the help tooltip.
 */
#include <iostream>

#include "delpi.h"

void OnSolve(const delpi::LpSolver& lp_solver, const delpi::LpResult result, const std::vector<mpq_class>& x,
             const std::vector<mpq_class>&, const mpq_class& obj_lb, const mpq_class& obj_ub, const mpq_class&) {
  // mpq_get_d() rounds towards 0.  This code guarantees infeas_gt > infeas.
  const mpq_class diff = obj_ub - obj_lb;
  // fmt::print uses shortest round-trip format for doubles, by default
  fmt::print("{} with delta = {} ( = {}), range = [{}, {}]", result, diff.get_d(), diff, obj_lb, obj_ub);
  if (lp_solver.config().with_timings()) fmt::println(" after {} seconds", lp_solver.stats().timer().seconds());
  if (lp_solver.config().produce_models()) fmt::println("Model: {}", lp_solver.model(x));
  std::cout << std::endl;
}

bool OnPartialSolve(const delpi::LpSolver& lp_solver, const delpi::LpResult result, const std::vector<mpq_class>& x,
                    const std::vector<mpq_class>&, const mpq_class& obj_lb, const mpq_class& obj_ub,
                    const mpq_class& diff, const mpq_class&) {
  fmt::print("PARTIAL: {} with delta = {} ( = {}), range = [{}, {}]", result, diff.get_d(), diff, obj_lb, obj_ub);
  if (lp_solver.config().with_timings()) fmt::println(" after {} seconds", lp_solver.stats().timer().seconds());
  if (lp_solver.config().produce_models()) fmt::println("Model: {}", lp_solver.model(x));
  std::cout << std::endl;
  return true;
}

int main(const int argc, const char* argv[]) {
  // Initialize the command line parser.
  delpi::ArgParser parser{};
  // Parse the command line arguments.
  parser.Parse(argc, argv);
  // Get the configuration from the command line arguments.
  const delpi::Config config = parser.ToConfig();

  // Setup the infinity values.
  const auto lp_solver{delpi::LpSolver::GetInstance(config)};
  lp_solver->m_solve_cb() = &OnSolve;
  lp_solver->m_partial_solve_cb() = &OnPartialSolve;

  if (!lp_solver->Parse()) {
    std::cerr << "Error parsing the input" << std::endl;
    return EXIT_FAILURE;
  }

  // Run the solver
  mpq_class precision{config.precision()};
  const delpi::LpResult result = lp_solver->Solve(precision);

  if (config.silent()) return ExitCode(result);

  // Print additional information about the result
  if (lp_solver->ConflictingExpected(result)) {
    std::cerr << "WARNING: Expected " << lp_solver->expected() << " but got " << result << std::endl;
  }
  if (config.verify() && IsFeasible(result)) {
    if (lp_solver->Verify())
      std::cout << "Model correctly satisfies the input" << std::endl;
    else
      std::cerr << "WARNING: Model does not satisfy the input" << std::endl;
  }

  return ExitCode(result);
}
