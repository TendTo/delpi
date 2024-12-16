/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/solver/LpSolver.h"
#include "delpi/util/filesystem.h"
#include "tests/solver/SolverUtils.h"

using delpi::Config;
using delpi::Expression;
using delpi::Formula;
using delpi::FormulaKind;
using delpi::GetFiles;
using delpi::LpResult;
using delpi::LpSolver;
using delpi::Variable;

class TestLpSolverMps : public ::testing::TestWithParam<std::tuple<Config::LpSolver, std::string, double>> {
 protected:
  Config config_;
  Variable x_{"x"}, y_{"y"}, z_{"z"};
  std::unique_ptr<LpSolver> solver_;

  TestLpSolverMps() {
    const auto& [solver, filename, precision] = GetParam();
    config_.m_format() = Config::Format::MPS;
    config_.m_lp_solver() = solver;
    config_.m_filename() = filename;
    config_.m_precision() = precision;
    solver_ = LpSolver::GetInstance(config_);
  }
};

INSTANTIATE_TEST_SUITE_P(TestDeltaMps, TestLpSolverMps,
                         ::testing::Combine(enabled_test_solvers, ::testing::ValuesIn(GetFiles("tests/solver/mps")),
                                            ::testing::Values(0.1)));

TEST_P(TestLpSolverMps, MpsInputAgainstExpectedOutput) {
  mpq_class precision = config_.precision();
  solver_->Parse();
  const LpResult result = solver_->Solve(precision);

  // Ignore the test if the solver is not supported or if it's too slow
  // if (result == LpResult::ERROR || result == LpResult::TIMEOUT) GTEST_SKIP();
  if (result == LpResult::ERROR) GTEST_SKIP();

  ASSERT_EQ(result, result == LpResult::DELTA_OPTIMAL ? LpResult::DELTA_OPTIMAL : solver_->expected());
  if (precision == 0) {
    ASSERT_TRUE(solver_->Verify());
  }
}
