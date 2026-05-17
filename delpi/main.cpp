/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 dlinear
 * @licence BSD 3-Clause License
 * Entry point of delpi.
 * Use the `-h` flag to show the help tooltip.
 */
#include <iostream>
#include <vector>

#include "delpi/delpi.h"
#include "delpi/util/error.h"

#define CSV_HEADER                                                                                                     \
  "file,solver,result,delta,actual_delta,precision,iterations,refinements,obj_lb,obj_ub,time_unit,parser_time,solver_" \
  "time,total_time"
#define CSV_FORMAT "{},{},{},{},{},{},{},{},{},{},s,{},{},{}"
#define CSV_PARTIAL_FORMAT "{},{},partial-{},{},{},{},{},{},{},{},s,{},{},{}"

delpi::Timer global_timer{};

void OnSolve(const delpi::LpSolver& lp_solver, const delpi::LpResult result, const std::vector<mpq_class>& x,
             const std::vector<mpq_class>&, const mpq_class& obj_lb, const mpq_class& obj_ub) {
  if (lp_solver.config().silent()) return;

  const mpq_class actual_delta = obj_ub - obj_lb;
  const delpi::Config& config = lp_solver.config();
  const delpi::LpStats& stats = lp_solver.stats();
  DELPI_ASSERT(actual_delta <= config.delta(), "Expected actual delta to be <= delta");

  if (config.csv()) {
    fmt::println(CSV_FORMAT, config.filename(), config.lp_solver(), result, config.delta(), actual_delta.get_d(),
                 stats.precision, stats.solver_stats.iterations(), stats.refinements, obj_lb.get_d(), obj_ub.get_d(),
                 stats.parser_stats.timer().seconds(), stats.solver_stats.timer().seconds(), global_timer.seconds());
    return;
  }
  switch (result) {
    case delpi::LpResult::OPTIMAL:
      DELPI_ASSERT(actual_delta == 0, "Expected actual delta to be 0");
      fmt::println("{}, objective value = {} ( = {})", result, obj_lb, obj_lb.get_d());
      break;
    case delpi::LpResult::DELTA_OPTIMAL:
      DELPI_ASSERT(actual_delta > 0, "Expected actual delta to be > 0");
      fmt::println("{} with delta = {} ( = {}), range = [{}, {}] ( = [{}, {}])", result, actual_delta.get_d(),
                   actual_delta, obj_lb, obj_ub, obj_lb.get_d(), obj_ub.get_d());
      break;
    default:
      fmt::println("{}", result);
  }
  if (config.with_timings()) {
    fmt::println("\tafter {} seconds\n{}\n{}", global_timer.seconds(), stats.parser_stats, stats.solver_stats);
  }
  if (config.produce_models()) fmt::println("Model: {}", lp_solver.model(x));
  std::cout << std::flush;
}

bool OnPartialSolve(const delpi::LpSolver& lp_solver, const delpi::LpResult result, const std::vector<mpq_class>& x,
                    const std::vector<mpq_class>&, const mpq_class& obj_lb, const mpq_class& obj_ub) {
  if (lp_solver.config().silent()) return true;

  const mpq_class actual_delta = obj_ub - obj_lb;
  const delpi::Config& config = lp_solver.config();
  const delpi::LpStats& stats = lp_solver.stats();
  DELPI_ASSERT(actual_delta > lp_solver.config().delta(), "Expected diff to be > delta");

  if (config.csv()) {
    fmt::println(CSV_PARTIAL_FORMAT, config.filename(), config.lp_solver(), result, actual_delta.get_d(),
                 actual_delta.get_d(), stats.precision, stats.solver_stats.iterations(), stats.refinements,
                 obj_lb.get_d(), obj_ub.get_d(), stats.parser_stats.timer().seconds(),
                 stats.solver_stats.timer().seconds(), global_timer.seconds());
    return true;
  }
  fmt::println("PARTIAL: {} with delta = {} ( = {}), range = [{}, {}]", result, actual_delta.get_d(), actual_delta,
               obj_lb, obj_ub);
  if (config.with_timings()) {
    fmt::println("\tafter {} seconds\n{}\n{}", global_timer.seconds(), stats.parser_stats, stats.solver_stats);
  }
  if (config.produce_models()) fmt::println("Model: {}", lp_solver.model(x));
  std::cout << std::flush;
  return true;
}

int main(const int argc, const char* argv[]) {
  // Initialize the command line parser.
  delpi::ArgParser parser{};
  // Parse the command line arguments.
  parser.Parse(argc, argv);
  // Get the configuration from the command line arguments.
  const delpi::Config config = parser.ToConfig();

  delpi::TimerGuard timer_guard{&global_timer, config.with_timings()};

  // Setup the infinity values.
  const auto lp_solver{delpi::LpSolver::GetInstance(config)};
  lp_solver->m_solve_cb() = &OnSolve;
  lp_solver->m_partial_solve_cb() = &OnPartialSolve;

  if (!lp_solver->Parse()) {
    std::cerr << "Error parsing the input" << std::endl;
    return EXIT_FAILURE;
  }

  // If csv output is enabled, print the header
  if (config.csv()) std::cout << CSV_HEADER << std::endl;

  // Run the solver
  mpq_class delta{config.delta()};
  const delpi::LpResult result = lp_solver->Solve();

  if (config.silent()) return ExitCode(result);

  // Print additional information about the result
  if (!lp_solver->CheckAgainstExpected(result)) {
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
