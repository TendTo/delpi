/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/solver/LpSolver.h"
#include "tests/TestUtils.h"

using delpi::Config;
using delpi::Expression;
using delpi::Formula;
using delpi::FormulaKind;
using delpi::LpResult;
using delpi::LpSolver;
using delpi::Variable;

class TestObjSense : public ::testing::TestWithParam<Config::LpSolver> {
 protected:
  Config config_;
  Variable x_{"x"}, y_{"y"}, z_{"z"};
  std::unique_ptr<LpSolver> solver_;

  TestObjSense() {
    config_.m_format() = Config::Format::MPS;
    config_.m_lp_solver() = GetParam();
    config_.m_verbose_simplex() = 3;
    solver_ = LpSolver::GetInstance(config_);
  }
};

INSTANTIATE_TEST_SUITE_P(TestObjSense, TestObjSense, enabled_test_solvers);

TEST_P(TestObjSense, Minimise) {
  solver_->AddColumn(x_, 16, 8, 15);
  solver_->AddRow(x_ >= 0);
  solver_->Minimise(2 * x_);

  ASSERT_EQ(solver_->Solve(), delpi::LpResult::OPTIMAL);
  ASSERT_EQ(solver_->obj_lb(), 16);
}

TEST_P(TestObjSense, Maximise) {
  solver_->AddColumn(x_, 16, 8, 15);
  solver_->AddRow(x_ >= 0);
  solver_->Maximise(2 * x_);

  ASSERT_EQ(solver_->Solve(), delpi::LpResult::OPTIMAL);
  ASSERT_EQ(solver_->obj_lb(), 30);
}

TEST_P(TestObjSense, MinimiseToMaximise) {
  solver_->AddColumn(x_, 16, 8, 15);
  solver_->AddRow(x_ >= 0);
  solver_->Minimise(2 * x_);
  solver_->Maximise(2 * x_);

  ASSERT_EQ(solver_->Solve(), delpi::LpResult::OPTIMAL);
  ASSERT_EQ(solver_->obj_lb(), 30);
}

TEST_P(TestObjSense, MaximiseToMinimise) {
  solver_->AddColumn(x_, 16, 8, 15);
  solver_->AddRow(x_ >= 0);
  solver_->Maximise(2 * x_);
  solver_->Minimise(2 * x_);

  ASSERT_EQ(solver_->Solve(), delpi::LpResult::OPTIMAL);
  ASSERT_EQ(solver_->obj_lb(), 16);
}
