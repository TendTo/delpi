/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "delpi/parser/mps/Driver.h"
#include "tests/TestUtils.h"

using delpi::Config;
using delpi::Formula;
using delpi::LpSolver;
using delpi::Variable;
using delpi::mps::MpsDriver;

class TestMpsDriver : public ::testing::TestWithParam<Config::LpSolver> {
 protected:
  Config config_{Config::Format::MPS};
  std::unique_ptr<LpSolver> lp_solver_;

  TestMpsDriver() {
    config_.m_lp_solver() = GetParam();
    lp_solver_ = LpSolver::GetInstance(config_);
  }
};

INSTANTIATE_TEST_SUITE_P(TestMpsDriver, TestMpsDriver, enabled_test_solvers);

TEST_P(TestMpsDriver, SetConfigOptions1) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("* @set-option :delta 1\n"
                         "* @set-option :produce-models true\n"
                         "ENDATA"));
  EXPECT_EQ(driver.config().delta(), 1);
  EXPECT_TRUE(driver.config().produce_models());
}

TEST_P(TestMpsDriver, SetConfigOptions2) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("* @set-option :delta 0.505\n"
                         "* @set-option :produce-models false\n"
                         "ENDATA"));
  EXPECT_EQ(driver.config().delta(), 0.505);
  EXPECT_FALSE(driver.config().produce_models());
}

TEST_P(TestMpsDriver, Name) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("NAME best name ever\n"
                         "ENDATA"));
  EXPECT_EQ(driver.problem_name(), "best name ever");
}

TEST_P(TestMpsDriver, Rows) {
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

TEST_P(TestMpsDriver, SimpleBoundsPositive) {
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

TEST_P(TestMpsDriver, NamesBoundAndRhs) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " L  R1\n"
                         " G  R2\n"
                         " E  R3\n"
                         " E  R4\n"  // ignored row
                         " N  Ob\n"  // only used for objective
                         "COLUMNS\n"
                         " X1 R1 1.\n"
                         " X2 R2 2.\n"
                         " X3 R3 3.\n"
                         "BOUNDS\n"
                         " FR BND X1\n"
                         " FR BND X2\n"
                         " FR BND X3\n"
                         "RHS\n"
                         " rhs1 R1 11\n"
                         " rhs2 R2 22 R3 33\n"
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

TEST_P(TestMpsDriver, SimpleBoundsNegative) {
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

TEST_P(TestMpsDriver, Columns) {
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

TEST_P(TestMpsDriver, Rhs) {
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

TEST_P(TestMpsDriver, RangePositive) {
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

TEST_P(TestMpsDriver, RangeNegative) {
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

TEST_P(TestMpsDriver, BoundsPositive) {
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
                         " X6 R1 1 \n"
                         "BOUNDS\n"
                         " LO BND X1 61\n"
                         " UP BND X2 62\n"
                         " FX BND X3 63\n"
                         " FR BND X4 64\n"
                         " MI BND X5 65\n"
                         " PL BND X5 66\n"
                         " SC BND X6 67\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 6u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const Variable& x4 = lp_solver_->variables().at(3);
  const Variable& x5 = lp_solver_->variables().at(4);
  const Variable& x6 = lp_solver_->variables().at(5);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(x1 >= 61,  //
                                              x2 >= 0,   //
                                              x2 <= 62,  //
                                              x3 == 63,  //
                                              x6 >= 0,   //
                                              x6 <= 67,  //
                                              x1 + x2 + x3 + x4 + x5 + x6 == 0));
}

TEST_P(TestMpsDriver, BoundsNegative) {
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
                         " X6 R1 1 \n"
                         "BOUNDS\n"
                         " LO BND X1 -61\n"
                         " UP BND X2 -62\n"
                         " FX BND X3 -63\n"
                         " FR BND X4 -64\n"
                         " MI BND X5 -65\n"
                         " PL BND X5 -66\n"
                         " SC BND X6 -67\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 6u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const Variable& x4 = lp_solver_->variables().at(3);
  const Variable& x5 = lp_solver_->variables().at(4);
  const Variable& x6 = lp_solver_->variables().at(5);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(x1 >= -61,  //
                                              x2 <= -62,  //
                                              x3 == -63,  //
                                              x6 <= -67,  //
                                              x1 + x2 + x3 + x4 + x5 + x6 == 0));
}

TEST_P(TestMpsDriver, BoundsExplicitMissingName) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " E  R1\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " X1 R1 1 \n"
                         " X2 R1 1 \n"
                         " X3 R1 1 \n"
                         "BOUNDS\n"
                         " LO X1 61\n"
                         " UP X2 62\n"
                         " FX X3 63\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 3u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(x1 >= 61,  //
                                              x2 >= 0,   //
                                              x2 <= 62,  //
                                              x3 == 63,  //
                                              x1 + x2 + x3 == 0));
}

TEST_P(TestMpsDriver, BoundsImplicit) {
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

TEST_P(TestMpsDriver, BoundsImplicitMissignName) {
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
                         " FR X4\n"
                         " MI X5\n"
                         " PL X5\n"
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

TEST_P(TestMpsDriver, BoundsIntegerImplicit) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " E  R1\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " X1 R1 1 \n"
                         " Mark 'MARKER' 'INTORG'\n"
                         " X2 R1 1 \n"
                         " X3 R1 1 \n"
                         " X4 R1 1 \n"
                         " X5 R1 1 \n"
                         " Mark 'MARKER' 'INTEND'\n"
                         " X6 R1 1 \n"
                         "BOUNDS\n"
                         " LO BND X2 -10\n"
                         " UP BND X4 10\n"
                         " UP BND X5 -1\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 6u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const Variable& x4 = lp_solver_->variables().at(3);
  const Variable& x5 = lp_solver_->variables().at(4);
  const Variable& x6 = lp_solver_->variables().at(5);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints, ::testing::UnorderedElementsAre(x1 >= 0,    //
                                                           x2 >= -10,  //
                                                           x2 <= 1,    //
                                                           x3 >= 0,    //
                                                           x3 <= 1,    //
                                                           x4 >= 0,    //
                                                           x4 <= 10,   //
                                                           x5 <= -1,   //
                                                           x6 >= 0,    //
                                                           x1 + x2 + x3 + x4 + x5 + x6 == 0));
}

TEST_P(TestMpsDriver, BoundsIntegerImplicitOnBounds) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " E  R1\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " X1 R1 1 \n"
                         " X2 R1 1 \n"
                         " Mark 'MARKER' 'INTORG'\n"
                         " X3 R1 1 \n"
                         " X4 R1 1 \n"
                         " Mark 'MARKER' 'INTEND'\n"
                         " X5 R1 1 \n"
                         " X6 R1 1 \n"
                         "BOUNDS\n"
                         " LI BND X2 -10\n"
                         " UP BND X4 10\n"
                         " UI BND X5 -1\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 6u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const Variable& x4 = lp_solver_->variables().at(3);
  const Variable& x5 = lp_solver_->variables().at(4);
  const Variable& x6 = lp_solver_->variables().at(5);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints, ::testing::UnorderedElementsAre(x1 >= 0,    //
                                                           x2 >= -10,  //
                                                           x3 >= 0,    //
                                                           x3 <= 1,    //
                                                           x4 >= 0,    //
                                                           x4 <= 10,   //
                                                           x5 <= -1,   //
                                                           x6 >= 0,    //
                                                           x1 + x2 + x3 + x4 + x5 + x6 == 0));
}

TEST_P(TestMpsDriver, BoundsLower) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " E  R1\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " Mark 'MARKER' 'INTORG'\n"
                         " X1 R1 1 \n"
                         " X2 R1 1 \n"
                         " X3 R1 1 \n"
                         " X4 R1 1 \n"
                         " Mark 'MARKER' 'INTEND'\n"
                         " X5 R1 1 \n"
                         " X6 R1 1 \n"
                         " X7 R1 1 \n"
                         " X8 R1 1 \n"
                         "BOUNDS\n"
                         " LO BND X1 -1\n"
                         " LO BND X2 1\n"
                         " LI BND X3 -1\n"
                         " LI BND X4 1\n"
                         " LO BND X5 -1\n"
                         " LO BND X6 1\n"
                         " LI BND X7 -1\n"
                         " LI BND X8 1\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 8u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const Variable& x4 = lp_solver_->variables().at(3);
  const Variable& x5 = lp_solver_->variables().at(4);
  const Variable& x6 = lp_solver_->variables().at(5);
  const Variable& x7 = lp_solver_->variables().at(6);
  const Variable& x8 = lp_solver_->variables().at(7);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints, ::testing::UnorderedElementsAre(x1 >= -1,  //
                                                           x1 <= 1,   //
                                                           x2 == 1,   //
                                                           x3 >= -1,  //
                                                           x4 >= 1,   //
                                                           x5 >= -1,  //
                                                           x6 >= 1,   //
                                                           x7 >= -1,  //
                                                           x8 >= 1,   //
                                                           x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 == 0));
}

TEST_P(TestMpsDriver, BoundsUpper) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " E  R1\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " Mark 'MARKER' 'INTORG'\n"
                         " X1 R1 1 \n"
                         " X2 R1 1 \n"
                         " X3 R1 1 \n"
                         " X4 R1 1 \n"
                         " Mark 'MARKER' 'INTEND'\n"
                         " X5 R1 1 \n"
                         " X6 R1 1 \n"
                         " X7 R1 1 \n"
                         " X8 R1 1 \n"
                         "BOUNDS\n"
                         " UP BND X1 -2\n"
                         " UP BND X2 2\n"
                         " UI BND X3 -2\n"
                         " UI BND X4 2\n"
                         " UP BND X5 -2\n"
                         " UP BND X6 2\n"
                         " UI BND X7 -2\n"
                         " UI BND X8 2\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 8u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const Variable& x2 = lp_solver_->variables().at(1);
  const Variable& x3 = lp_solver_->variables().at(2);
  const Variable& x4 = lp_solver_->variables().at(3);
  const Variable& x5 = lp_solver_->variables().at(4);
  const Variable& x6 = lp_solver_->variables().at(5);
  const Variable& x7 = lp_solver_->variables().at(6);
  const Variable& x8 = lp_solver_->variables().at(7);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints, ::testing::UnorderedElementsAre(x1 <= -2,  //
                                                           x2 <= 2,   //
                                                           x2 >= 0,   //
                                                           x3 <= -2,  //
                                                           x4 <= 2,   //
                                                           x4 >= 0,   //
                                                           x5 <= -2,  //
                                                           x6 <= 2,   //
                                                           x6 >= 0,   //
                                                           x7 <= -2,  //
                                                           x8 <= 2,   //
                                                           x8 >= 0,   //
                                                           x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 == 0));
}

TEST_P(TestMpsDriver, RangesDefaultRhs) {
  MpsDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("ROWS\n"
                         " L  R1\n"
                         " E  R2\n"
                         " E  R3\n"
                         " G  R4\n"
                         " N  Ob\n"
                         "COLUMNS\n"
                         " X1 R1 1 \n"
                         " X1 R2 1 \n"
                         " X1 R3 1 \n"
                         " X1 R4 1 \n"
                         " X1 Ob 1 \n"
                         "BOUNDS\n"
                         " FR BND X1\n"
                         "RANGES\n"
                         " RNG R1 1\n"
                         " RNG R2 2\n"
                         " RNG R3 -3\n"
                         " RNG R4 4\n"
                         "ENDATA"));
  ASSERT_EQ(lp_solver_->variables().size(), 1u);
  const Variable& x1 = lp_solver_->variables().at(0);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints, ::testing::UnorderedElementsAre(x1 <= 0,   //
                                                           x1 >= -1,  //
                                                           x1 <= 2,   //
                                                           x1 >= 0,   //
                                                           x1 >= -3,  //
                                                           x1 <= 0,   //
                                                           x1 >= 0,   //
                                                           x1 <= 4));
}
