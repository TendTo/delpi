/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "delpi/solver/DelpiLpSolver.h"
#include "delpi/util/Config.h"

using delpi::Config;
using delpi::DelpiLpSolver;
using delpi::LpResult;
using delpi::LpSolver;
using delpi::Variable;

class TestDelpiLpSolver : public ::testing::Test {
 protected:
  std::unique_ptr<LpSolver> solver_{std::make_unique<DelpiLpSolver>()};
  Variable x1_{"x1"}, x2_{"x2"}, x3_{"x3"}, x4_{"x4"}, x5_{"x5"}, x6_{"x6"}, x7_{"x7"}, x8_{"x8"}, x9_{"x9"};
  mpq_class delta_{0};
};

TEST_F(TestDelpiLpSolver, Constructor) {
  const auto* solver = dynamic_cast<const DelpiLpSolver*>(solver_.get());
  ASSERT_NE(solver, nullptr);
  EXPECT_EQ(solver->b().size(), 0u);
  EXPECT_EQ(solver->c().size(), 0u);
  EXPECT_EQ(solver->num_columns(), 0);
  EXPECT_EQ(solver->num_rows(), 0);
  EXPECT_EQ(solver->A().rows(), 0);
  EXPECT_EQ(solver->A().cols(), 0);
  EXPECT_EQ(solver->x().size(), 0);
}

TEST_F(TestDelpiLpSolver, Feasible) {
  solver_->AddColumn(x1_, 0);
  solver_->AddColumn(x2_, 0);
  solver_->AddColumn(x3_, 0);
  solver_->AddColumn(x4_, 0);
  solver_->AddRow(x1_ + x2_ + x3_ + x4_ <= 1);
  solver_->AddRow(x1_ + x2_ + x3_ + x4_ >= 1);
  solver_->m_solve_cb() = [&](const LpSolver&, const LpResult, const std::vector<mpq_class>& x,
                              const std::vector<mpq_class>&, const mpq_class& obj_lb, const mpq_class& obj_ub,
                              const mpq_class& delta) {
    EXPECT_EQ(obj_lb, 0);
    EXPECT_EQ(obj_ub, 0);
    EXPECT_EQ(x.size(), 4u);
    EXPECT_THAT(x, ::testing::Each(::testing::AllOf(::testing::Eq(0))));
    EXPECT_LE(delta, delta_);
  };
  const LpResult result = solver_->Solve(delta_);
  EXPECT_EQ(result, LpResult::OPTIMAL);
}

TEST_F(TestDelpiLpSolver, Infeasible) {
  solver_->AddColumn(x1_, 0);
  solver_->AddColumn(x2_, 0);
  solver_->AddColumn(x3_, 0);
  solver_->AddColumn(x4_, 0);
  solver_->AddRow(x1_ + x2_ + x3_ + x4_ <= -1);
  solver_->AddRow(x1_ + x2_ + x3_ + x4_ >= -10);
  const LpResult result = solver_->Solve(delta_);
  EXPECT_EQ(result, LpResult::INFEASIBLE);
}

TEST_F(TestDelpiLpSolver, EqualityOptimalExample) {
  solver_->AddColumn(x1_, -1);
  solver_->AddColumn(x2_, -2);
  solver_->AddColumn(x3_, 1);
  solver_->AddRow(2 * x1_ + x2_ + x3_ == 14);
  solver_->AddRow(4 * x1_ + 2 * x2_ + 3 * x3_ == 28);
  solver_->AddRow(2 * x1_ + 5 * x2_ + 5 * x3_ == 30);
  solver_->m_solve_cb() = [&](const LpSolver&, const LpResult, const std::vector<mpq_class>& x,
                              const std::vector<mpq_class>&, const mpq_class& obj_lb, const mpq_class& obj_ub,
                              const mpq_class& delta) {
    EXPECT_EQ(obj_lb, -13);
    EXPECT_EQ(obj_ub, -13);
    EXPECT_EQ(x.size(), 3u);
    EXPECT_THAT(x, ::testing::Each(::testing::AllOf(::testing::Ge(0))));
    EXPECT_LE(delta, delta_);
  };
  const LpResult result = solver_->Solve(delta_);
  EXPECT_EQ(result, LpResult::OPTIMAL);
}

TEST_F(TestDelpiLpSolver, UnboundedExample) {
  solver_->AddColumn(x1_, -1);
  solver_->AddColumn(x2_, -2);
  solver_->AddColumn(x3_, 1);
  solver_->AddRow(2 * x1_ + x2_ + x3_ >= 14);
  solver_->AddRow(4 * x1_ + 2 * x2_ + 3 * x3_ >= 28);
  solver_->AddRow(2 * x1_ + 5 * x2_ + 5 * x3_ >= 30);
  const LpResult result = solver_->Solve(delta_);
  EXPECT_EQ(result, LpResult::UNBOUNDED);
}

TEST_F(TestDelpiLpSolver, InfeasibleExample) {
  solver_->AddColumn(x1_, -1);
  solver_->AddColumn(x2_, -2);
  solver_->AddColumn(x3_, 1);
  solver_->AddRow(2 * x1_ + x2_ + x3_ == -14);
  solver_->AddRow(4 * x1_ + 2 * x2_ + 3 * x3_ == 28);
  solver_->AddRow(2 * x1_ + 5 * x2_ + 5 * x3_ == 30);
  const LpResult result = solver_->Solve(delta_);
  EXPECT_EQ(result, LpResult::INFEASIBLE);
}

TEST_F(TestDelpiLpSolver, ContraintSignSensesCombination) {
  solver_->AddColumn(x1_, -1);
  solver_->AddColumn(x2_, -2);
  solver_->AddColumn(x3_, 1);
  solver_->AddColumn(x4_, 5);
  solver_->AddColumn(x5_, 2);
  solver_->AddColumn(x6_, 4);
  solver_->AddColumn(x7_, 6);
  solver_->AddColumn(x8_, -5);
  solver_->AddColumn(x9_, -7);

  solver_->AddRow(2 * x1_ + x2_ + x3_ == 14);
  solver_->AddRow(-4 * x7_ - 2 * x8_ + 3 * x3_ == -1);
  solver_->AddRow(2 * x1_ + 5 * x2_ + 5 * x3_ >= 30);
  solver_->AddRow(2 * x1_ + x2_ + x3_ >= -14);
  solver_->AddRow(4 * x5_ + 2 * x6_ + 3 * x3_ <= 28);
  solver_->AddRow(2 * x1_ + 5 * x2_ - 5 * x9_ <= -5);
  const LpResult result = solver_->Solve(delta_);
  EXPECT_EQ(result, LpResult::UNBOUNDED);
}
