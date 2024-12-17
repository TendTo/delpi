/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "delpi/parser/mps/Driver.h"

using delpi::Config;
using delpi::Formula;
using delpi::LpSolver;
using delpi::Variable;
using delpi::mps::MpsDriver;

class TestMpsDriver : public ::testing::Test {
 protected:
  Config config_{Config::Format::MPS};
  std::unique_ptr<LpSolver> lp_solver_;

  TestMpsDriver() {
    config_.m_lp_solver() = Config::LpSolver::SOPLEX;
    lp_solver_ = LpSolver::GetInstance(config_);
  }
};

TEST_F(TestMpsDriver, SetConfigOptions1) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("* @set-option :precision 1\n"
                         "* @set-option :produce-models true\n"
                         "ENDATA"));
  EXPECT_EQ(driver.config().precision(), 1);
  EXPECT_TRUE(driver.config().produce_models());
}

TEST_F(TestMpsDriver, SetConfigOptions2) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("* @set-option :precision 0.505\n"
                         "* @set-option :produce-models false\n"
                         "ENDATA"));
  EXPECT_EQ(driver.config().precision(), 0.505);
  EXPECT_FALSE(driver.config().produce_models());
}

TEST_F(TestMpsDriver, Name) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("NAME best name ever\n"
                         "ENDATA"));
  EXPECT_EQ(driver.problem_name(), "best name ever");
}

TEST_F(TestMpsDriver, Rows) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " L  R1\n"
                         " G  R2\n"
                         " E  R3\n"
                         " E  R4\n"  // ignored row
                         " N  Ob\n"  // only used for  objective
                         "COLUMNS\n"
                         " X1 R1 1.\n"
                         " X2 R2 2.\n"
                         " X3 R3 3.\n"
                         " X4 Ob 4.\n"
                         "BOUNDS\n"
                         " FR BND X1\n"
                         " FR BND X2\n"
                         " FR BND X3\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 4u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const Variable& x4 = lp_solver_->variables().at(3);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints, ::testing::UnorderedElementsAre(x1 <= 0,      //
                                                           2 * x2 >= 0,  //
                                                           3 * x3 == 0,  //
                                                           x4 >= 0));
}

TEST_F(TestMpsDriver, SimpleBoundsPositive) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " L  R1\n"
                         " G  R2\n"
                         " E  R3\n"
                         " E  R4\n"  // ignored row
                         " N  Ob\n"  // only used for  objective
                         "COLUMNS\n"
                         " X1 R1 1.\n"
                         " X2 R2 2.\n"
                         " X3 R3 3.\n"
                         "BOUNDS\n"
                         " FR BND X1\n"
                         " FR BND X2\n"
                         " FR BND X3\n"
                         "RHS\n"
                         " R1 11\n"
                         " R2 22 R3 33\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 3u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints, ::testing::UnorderedElementsAre(x1 <= 11,      //
                                                           2 * x2 >= 22,  //
                                                           3 * x3 == 33));
}

TEST_F(TestMpsDriver, SimpleBoundsNegative) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " L  R1\n"
                         " G  R2\n"
                         " E  R3\n"
                         " E  R4\n"  // ignored row
                         " N  Ob\n"  // only used for  objective
                         "COLUMNS\n"
                         " X1 R1 -1.\n"
                         " X2 R2 -2.\n"
                         " X3 R3 -3.\n"
                         "BOUNDS\n"
                         " FR BND X1\n"
                         " FR BND X2\n"
                         " FR BND X3\n"
                         "RHS\n"
                         " R1 11\n"
                         " R2 22 R3 33\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 3u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints, ::testing::UnorderedElementsAre(-1 * x1 <= 11,  //
                                                           -2 * x2 >= 22,  //
                                                           -3 * x3 == 33));
}

TEST_F(TestMpsDriver, Columns) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " L  R1\n"
                         " G  R2\n"
                         " E  R3\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " X1 R1 11 R2 12.0 \n"
                         " X2 R2 21.00 \n"
                         " X3 R1 31/1 R2 32 \n"
                         " X3 R3 33  \n"
                         "BOUNDS\n"
                         " FR BND X1\n"
                         " FR BND X2\n"
                         " FR BND X3\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 3u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(11 * x1 + 31 * x3 <= 0,            //
                                              12 * x1 + 21 * x2 + 32 * x3 >= 0,  //
                                              33 * x3 == 0));
}

TEST_F(TestMpsDriver, Rhs) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " L  R1\n"
                         " G  R2\n"
                         " E  R3\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " X1 R1 11 R2 12.0 \n"
                         " X2 R2 21.00 \n"
                         " X3 R1 31/1 R2 32 \n"
                         " X3 R3 33  \n"
                         "RHS\n"
                         " R1 1\n"
                         " R2 2 R3 3\n"
                         "BOUNDS\n"
                         " FR BND X1\n"
                         " FR BND X2\n"
                         " FR BND X3\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 3u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(11 * x1 + 31 * x3 <= 1,            //
                                              12 * x1 + 21 * x2 + 32 * x3 >= 2,  //
                                              33 * x3 == 3));
}

TEST_F(TestMpsDriver, RangePositive) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " L  R1\n"
                         " G  R2\n"
                         " E  R3\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " X1 R1 11 R2 12.0 \n"
                         " X2 R2 21.00 \n"
                         " X3 R1 31/1 R2 32 \n"
                         " X3 R3 33 \n"
                         "RHS\n"
                         " R1 1\n"
                         " R2 2 R3 3\n"
                         "RANGES\n"
                         " RNG R1 51\n"
                         " RNG R2 52 R3 53\n"
                         "BOUNDS\n"
                         " FR BND X1\n"
                         " FR BND X2\n"
                         " FR BND X3\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 3u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(11 * x1 + 31 * x3 >= 1 - 51,            //
                                              11 * x1 + 31 * x3 <= 1,                 //
                                              12 * x1 + 21 * x2 + 32 * x3 >= 2,       //
                                              12 * x1 + 21 * x2 + 32 * x3 <= 2 + 52,  //
                                              33 * x3 >= 3,                           //
                                              33 * x3 <= 3 + 53));
}

TEST_F(TestMpsDriver, RangeNegative) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " L  R1\n"
                         " G  R2\n"
                         " E  R3\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " X1 R1 11 R2 12.0 \n"
                         " X2 R2 21.00 \n"
                         " X3 R1 31/1 R2 32 \n"
                         " X3 R3 33 \n"
                         "RHS\n"
                         " R1 1\n"
                         " R2 2 R3 3\n"
                         "RANGES\n"
                         " RNG R1 -51\n"
                         " RNG R2 -52 R3 -53\n"
                         "BOUNDS\n"
                         " FR BND X1\n"
                         " FR BND X2\n"
                         " FR BND X3\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 3u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(11 * x1 + 31 * x3 >= 1 - 51,            //
                                              11 * x1 + 31 * x3 <= 1,                 //
                                              12 * x1 + 21 * x2 + 32 * x3 >= 2,       //
                                              12 * x1 + 21 * x2 + 32 * x3 <= 2 + 52,  //
                                              33 * x3 >= 3 - 53,                      //
                                              33 * x3 <= 3));
}

TEST_F(TestMpsDriver, BoundsPositive) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " E  R1\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " X1 R1 1 \n"
                         " X2 R1 1 \n"
                         " X3 R1 1 \n"
                         " X4 R1 1 \n"
                         " X5 R1 1 \n"
                         "BOUNDS\n"
                         " LO BND X1 61\n"
                         " UP BND X2 62\n"
                         " FX BND X3 63\n"
                         " FR BND X4 64\n"
                         " MI BND X5 65\n"
                         " PL BND X5 66\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 5u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const Variable& x4 = lp_solver_->variables().at(3);
  const Variable& x5 = lp_solver_->variables().at(4);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(x1 >= 61,  //
                                              x2 >= 0,   //
                                              x2 <= 62,  //
                                              x3 == 63,  //
                                              x1 + x2 + x3 + x4 + x5 == 0));
}

TEST_F(TestMpsDriver, BoundsNegative) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " E  R1\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " X1 R1 1 \n"
                         " X2 R1 1 \n"
                         " X3 R1 1 \n"
                         " X4 R1 1 \n"
                         " X5 R1 1 \n"
                         "BOUNDS\n"
                         " LO BND X1 -61\n"
                         " UP BND X2 -62\n"
                         " FX BND X3 -63\n"
                         " FR BND X4 -64\n"
                         " MI BND X5 -65\n"
                         " PL BND X5 -66\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 5u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const Variable& x4 = lp_solver_->variables().at(3);
  const Variable& x5 = lp_solver_->variables().at(4);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(x1 >= -61,  //
                                              x2 <= -62,  //
                                              x3 == -63,  //
                                              x1 + x2 + x3 + x4 + x5 == 0));
}

TEST_F(TestMpsDriver, BoundsImplicit) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " E  R1\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " X1 R1 1 \n"
                         " X2 R1 1 \n"
                         " X3 R1 1 \n"
                         " X4 R1 1 \n"
                         " X5 R1 1 \n"
                         "BOUNDS\n"
                         " FR BND X4\n"
                         " MI BND X5\n"
                         " PL BND X5\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 5u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const Variable& x4 = lp_solver_->variables().at(3);
  const Variable& x5 = lp_solver_->variables().at(4);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints, ::testing::UnorderedElementsAre(x1 >= 0,  //
                                                           x2 >= 0,  //
                                                           x3 >= 0,  //
                                                           x1 + x2 + x3 + x4 + x5 == 0));
}
