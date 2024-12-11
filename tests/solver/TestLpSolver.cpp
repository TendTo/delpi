/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/solver/LpSolver.h"
#include "tests/solver/SolverUtils.h"

using delpi::Config;
using delpi::Expression;
using delpi::Formula;
using delpi::FormulaKind;
using delpi::LpResult;
using delpi::LpSolver;
using delpi::Variable;

class TestLpSolver : public ::testing::TestWithParam<Config::LPSolver> {
 protected:
  Config config_;
  Variable x_{"x"}, y_{"y"}, z_{"z"};
  std::unique_ptr<LpSolver> solver_;

  TestLpSolver() {
    config_.m_format() = Config::Format::MPS;
    config_.m_lp_solver() = GetParam();
    config_.m_verbose_simplex() = 3;
    solver_ = LpSolver::GetInstance(config_);
  }
};

INSTANTIATE_TEST_SUITE_P(TestLpSolver, TestLpSolver, enabled_test_solvers);

TEST_P(TestLpSolver, Constructor) {
  EXPECT_LT(solver_->ninfinity(), 0);
  EXPECT_GT(solver_->infinity(), 0);
  EXPECT_EQ(&solver_->config(), &config_);
}

TEST_P(TestLpSolver, AddColumn) {
  const int col_idx = solver_->AddColumn(x_);

  EXPECT_TRUE(solver_->var(col_idx).equal_to(x_));
  EXPECT_TRUE(solver_->column(col_idx).var.equal_to(x_));
  EXPECT_EQ(solver_->column(col_idx).lb.value(), 0);
  EXPECT_FALSE(solver_->column(col_idx).ub.has_value());
  EXPECT_FALSE(solver_->column(col_idx).obj.has_value());
}
TEST_P(TestLpSolver, AddColumnObjective) {
  const int col_idx = solver_->AddColumn(x_, 17);

  EXPECT_TRUE(solver_->var(col_idx).equal_to(x_));
  EXPECT_TRUE(solver_->column(col_idx).var.equal_to(x_));
  EXPECT_EQ(solver_->column(col_idx).lb.value(), 0);
  EXPECT_FALSE(solver_->column(col_idx).ub.has_value());
  EXPECT_EQ(solver_->column(col_idx).obj.value(), 17);
}
TEST_P(TestLpSolver, AddColumnBounded) {
  const int col_idx = solver_->AddColumn(x_, 7, 8);

  EXPECT_TRUE(solver_->var(col_idx).equal_to(x_));
  EXPECT_TRUE(solver_->column(col_idx).var.equal_to(x_));
  EXPECT_EQ(solver_->column(col_idx).lb.value(), 7);
  EXPECT_EQ(solver_->column(col_idx).ub.value(), 8);
  EXPECT_FALSE(solver_->column(col_idx).obj.has_value());
}
TEST_P(TestLpSolver, AddColumnComplete) {
  const int col_idx = solver_->AddColumn(x_, 16, 8, 15);

  EXPECT_TRUE(solver_->var(col_idx).equal_to(x_));
  EXPECT_TRUE(solver_->column(col_idx).var.equal_to(x_));
  EXPECT_EQ(solver_->column(col_idx).lb.value(), 8);
  EXPECT_EQ(solver_->column(col_idx).ub.value(), 15);
  EXPECT_EQ(solver_->column(col_idx).obj.value(), 16);
}

TEST_P(TestLpSolver, AddRow) {
  const Formula f{x_ + 2 * y_, FormulaKind::Leq, 5};
  solver_->AddColumn(x_);
  solver_->AddColumn(y_);
  const int row_idx = solver_->AddRow(f);
  ASSERT_EQ(solver_->row(row_idx).addends.size(), 2u);
  for (const auto& [var, coeff] : solver_->row(row_idx).addends) {
    EXPECT_EQ(f.expression().addends().at(var), coeff);
  }
  EXPECT_FALSE(solver_->row(row_idx).lb.has_value());
  EXPECT_EQ(solver_->row(row_idx).ub.value(), 5);
}

#if 0
TEST_P(TestLpSolver, SetObjective) {
  Variable x{"x"};
  Variable y{"y"};
  solver_->AddColumn(x);
  solver_->AddColumn(y);
  solver_->SetObjective(x, 1);
  solver_->SetObjective(y, 2);
  EXPECT_EQ(solver_->solution().size(), 2);
}
#endif

TEST_P(TestLpSolver, Optimise) {
  solver_->AddColumn(x_, 9);
  solver_->AddColumn(y_, 1);
  solver_->AddRow(x_ + y_, FormulaKind::Geq, 10);
  mpq_class precision{0};
  const LpResult result = solver_->Optimise(precision);
  EXPECT_EQ(result, delpi::LpResult::OPTIMAL);
  EXPECT_EQ(solver_->solution(x_), 0);
  EXPECT_EQ(solver_->solution(y_), 10);
}

#ifndef NDEBUG
TEST_P(TestLpSolver, Dump) { EXPECT_NO_THROW(solver_->Dump()); }
#endif
