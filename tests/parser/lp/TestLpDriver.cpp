/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "delpi/parser/lp/Driver.h"
#include "tests/TestUtils.h"

using delpi::Config;
using delpi::Formula;
using delpi::LpSolver;
using delpi::Variable;
using delpi::lp::LpDriver;

class TestLpDriver : public ::testing::TestWithParam<Config::LpSolver> {
 protected:
  Config config_{Config::Format::LP};
  std::unique_ptr<LpSolver> lp_solver_;

  TestLpDriver() {
    DELPI_LOG_INIT_VERBOSITY(5);
    config_.m_lp_solver() = GetParam();
    lp_solver_ = LpSolver::GetInstance(config_);
  }
};

INSTANTIATE_TEST_SUITE_P(TestLpDriver, TestLpDriver, enabled_test_solvers);

TEST_P(TestLpDriver, SetConfigOptions1) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("\\ Comment \n"
                         "\\ @set-option :produce-models true\n"
                         "END"));
  EXPECT_TRUE(driver.config().produce_models());
}

TEST_P(TestLpDriver, SetConfigOptions2) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("\\ Comments \n"
                         "\\ @set-option :delta 0.505\n"
                         "\\ @set-option :produce-models false"));
  EXPECT_EQ(driver.config().delta(), 0.505);
  EXPECT_FALSE(driver.config().produce_models());
}

TEST_P(TestLpDriver, Name) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Problem\n"
                         " best name ever"));
  EXPECT_EQ(driver.problem_name(), "best name ever");
}

TEST_P(TestLpDriver, MinVariations) {
  for (const std::string_view min_variation : {"Minimize", "minimize", "MINIMIZE", "Min", "min", "MIN", "minimum"}) {
    LpDriver driver{*lp_solver_};
    ASSERT_TRUE(driver.ParseString(min_variation));
    ASSERT_EQ(driver.is_min(), true);
  }
}

TEST_P(TestLpDriver, MaxVariations) {
  for (const std::string_view max_variation : {"Maximize", "maximize", "MAXIMIZE", "Max", "max", "MAX", "maximum"}) {
    {
      LpDriver driver{*lp_solver_};
      ASSERT_TRUE(driver.ParseString(max_variation));
      ASSERT_EQ(driver.is_min(), false);
    }
  }
}

TEST_P(TestLpDriver, ObjectiveParserOneLine) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " z + x1 - y1 +2 x2 -2 y2 + 3x3 - 3y3 +4x4 -4y4 +.5x5 -.5y5 +6/6x6 -6/6y6 +7/7x7 -7/7y7\n"
                         "End"));
  ASSERT_EQ(lp_solver_->variables().size(), 15u);
}

TEST_P(TestLpDriver, ObjectiveParserOneLineNamed) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " objname: z + x1 - y1 +2 x2 -2 y2 + 3x3 - 3y3 +4x4 -4y4 +.5x5 -.5y5\n"
                         "End"));
  ASSERT_EQ(lp_solver_->variables().size(), 11u);
  ASSERT_EQ(driver.objective_name(), "objname");
}

TEST_P(TestLpDriver, ObjectiveParserOneLineNamedSpaced) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " objname : z + x1 - y1 +2 x2 -2 y2 + 3x3 - 3y3 +4x4 -4y4 +.5x5 -.5y5\n"
                         "End"));
  ASSERT_EQ(lp_solver_->variables().size(), 11u);
  ASSERT_EQ(driver.objective_name(), "objname");
}

TEST_P(TestLpDriver, ObjectiveParserMultiLine) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " z + x1 - y1 +2 x2 -2 y2 + 3x3 - 3y3\n"
                         " +4x4 -4y4 +.5x5 -.5y5 +6/6x6  \n"
                         " -6/6y6 +7/7x7 -7/7y7\n"
                         "End"));
  ASSERT_EQ(lp_solver_->variables().size(), 15u);
}

TEST_P(TestLpDriver, ObjectiveParserLineLineNamed) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " objname: z + x1 - y1 +2 x2 \n"
                         " -2 y2 + 3x3 - 3y3 +4x4\n"
                         " -4y4 +.5x5 -.5y5\n"
                         "End"));
  ASSERT_EQ(lp_solver_->variables().size(), 11u);
  ASSERT_EQ(driver.objective_name(), "objname");
}

TEST_P(TestLpDriver, ObjectiveParserMultiLineNamedSpaced) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " objname : z + x1 \n"
                         " - y1 +2 x2 -2 y2 + 3x3\n"
                         " - 3y3 +4x4 -4y4 +.5x5 -.5y5\n"
                         "End"));
  ASSERT_EQ(lp_solver_->variables().size(), 11u);
  ASSERT_EQ(driver.objective_name(), "objname");
}
TEST_P(TestLpDriver, ObjectiveEmpty) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         "End"));
  ASSERT_EQ(lp_solver_->variables().size(), 0u);
}

TEST_P(TestLpDriver, SimpleConstraintsNoNames) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         "st\n"
                         " x1 - x1n <= 0 \n"
                         " +2 x2 -2 x2n < 1.1\n"
                         " + 3x3 - 3x3n = 2.e2 \n"
                         " +4x4 \n"
                         " -4x4n +.5x5\n"
                         " -.5x5n +6/6x6 -6/6x6n > .3\n"
                         " +7/7x7 -7/7x7n >= 4/4\n"
                         "End"));
  constexpr std::size_t n_variables = 14;
  ASSERT_EQ(lp_solver_->variables().size(), n_variables);
  std::array<Variable, n_variables> vs;
  for (std::size_t i = 0; i < vs.size(); ++i) vs[i] = lp_solver_->variables().at(i);
  std::ranges::sort(vs, {}, &Variable::name);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(
                  vs[0] - vs[1] <= 0,                          //
                  2 * vs[2] - 2 * vs[3] <= mpq_class{11, 10},  //
                  3 * vs[4] - 3 * vs[5] == 200,                //
                  4 * vs[6] - 4 * vs[7] + mpq_class{1, 2} * vs[8] - mpq_class{1, 2} * vs[9] + 1 * vs[10] - 1 * vs[11] >=
                      mpq_class{3, 10},  //
                  1 * vs[12] - 1 * vs[13] >= 1,
                  vs[0] >= 0,   //
                  vs[1] >= 0,   //
                  vs[2] >= 0,   //
                  vs[3] >= 0,   //
                  vs[4] >= 0,   //
                  vs[5] >= 0,   //
                  vs[6] >= 0,   //
                  vs[7] >= 0,   //
                  vs[8] >= 0,   //
                  vs[9] >= 0,   //
                  vs[10] >= 0,  //
                  vs[11] >= 0,  //
                  vs[12] >= 0,  //
                  vs[13] >= 0));
}

TEST_P(TestLpDriver, SimpleConstraintsMixedNames) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         "st\n"
                         " c1: x1 - x1n <= 0 \n"
                         " +2 x2 -2 x2n < 1.1\n"
                         " c3  :  + 3x3 - 3x3n = 2.e2 \n"
                         " +4x4 \n"
                         " -4x4n +.5x5\n"
                         " c4: -.5x5n +6/6x6 -6/6x6n > .3\n"
                         " +7/7x7 -7/7x7n >= 4/4\n"
                         "End"));
  constexpr std::size_t n_variables = 14;
  ASSERT_EQ(lp_solver_->variables().size(), n_variables);
  std::array<Variable, n_variables> vs;
  for (std::size_t i = 0; i < vs.size(); ++i) vs[i] = lp_solver_->variables().at(i);
  std::ranges::sort(vs, {}, &Variable::name);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(
                  vs[0] - vs[1] <= 0,                          //
                  2 * vs[2] - 2 * vs[3] <= mpq_class{11, 10},  //
                  3 * vs[4] - 3 * vs[5] == 200,                //
                  4 * vs[6] - 4 * vs[7] + mpq_class{1, 2} * vs[8] - mpq_class{1, 2} * vs[9] + 1 * vs[10] - 1 * vs[11] >=
                      mpq_class{3, 10},  //
                  1 * vs[12] - 1 * vs[13] >= 1,
                  vs[0] >= 0,   //
                  vs[1] >= 0,   //
                  vs[2] >= 0,   //
                  vs[3] >= 0,   //
                  vs[4] >= 0,   //
                  vs[5] >= 0,   //
                  vs[6] >= 0,   //
                  vs[7] >= 0,   //
                  vs[8] >= 0,   //
                  vs[9] >= 0,   //
                  vs[10] >= 0,  //
                  vs[11] >= 0,  //
                  vs[12] >= 0,  //
                  vs[13] >= 0));
}

TEST_P(TestLpDriver, SimpleConstraintsMixedNamesNoSpaces) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         "st\n"
                         "c1: x1 - x1n <= 0 \n"
                         "+2 x2 -2 x2n < 1.1\n"
                         "c3  :  + 3x3 - 3x3n = 2.e2 \n"
                         "+4x4 \n"
                         "-4x4n +.5x5\n"
                         "c4: -.5x5n +6/6x6 -6/6x6n > .3\n"
                         "+7/7x7 -7/7x7n >= 4/4\n"
                         "End"));
  constexpr std::size_t n_variables = 14;
  ASSERT_EQ(lp_solver_->variables().size(), n_variables);
  std::array<Variable, n_variables> vs;
  for (std::size_t i = 0; i < vs.size(); ++i) vs[i] = lp_solver_->variables().at(i);
  std::ranges::sort(vs, {}, &Variable::name);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(
                  vs[0] - vs[1] <= 0,                          //
                  2 * vs[2] - 2 * vs[3] <= mpq_class{11, 10},  //
                  3 * vs[4] - 3 * vs[5] == 200,                //
                  4 * vs[6] - 4 * vs[7] + mpq_class{1, 2} * vs[8] - mpq_class{1, 2} * vs[9] + 1 * vs[10] - 1 * vs[11] >=
                      mpq_class{3, 10},  //
                  1 * vs[12] - 1 * vs[13] >= 1,
                  vs[0] >= 0,   //
                  vs[1] >= 0,   //
                  vs[2] >= 0,   //
                  vs[3] >= 0,   //
                  vs[4] >= 0,   //
                  vs[5] >= 0,   //
                  vs[6] >= 0,   //
                  vs[7] >= 0,   //
                  vs[8] >= 0,   //
                  vs[9] >= 0,   //
                  vs[10] >= 0,  //
                  vs[11] >= 0,  //
                  vs[12] >= 0,  //
                  vs[13] >= 0));
}

TEST_P(TestLpDriver, Bounds) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         "st\n"
                         "bounds\n"
                         " x1 <= 1\n"
                         " x2 < 2\n"
                         " x3 <= -3\n"
                         " x4 = 4\n"
                         " 5 <= x5\n"
                         " 6 < x6\n"
                         " 7 < x7 < 7\n"
                         " -8 < x8 < 8\n"
                         " -inf < x9 < inf\n"
                         " -infinity < x10\n"
                         " x11 <= inf\n"
                         " x12 free\n"
                         "End"));
  constexpr std::size_t n_variables = 12;
  ASSERT_EQ(lp_solver_->variables().size(), n_variables);
  std::array<Variable, n_variables> vs;
  for (std::size_t i = 0; i < vs.size(); ++i) vs[i] = lp_solver_->variables().at(i);
  std::ranges::sort(vs, {}, &Variable::id);
  fmt::println("{}", vs);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(vs[0] >= 0,   //
                                              vs[0] <= 1,   //
                                              vs[1] >= 0,   //
                                              vs[1] <= 2,   //
                                              vs[2] <= -3,  //
                                              vs[3] == 4,   //
                                              vs[4] >= 5,   //
                                              vs[5] >= 6,   //
                                              vs[6] == 7,   //
                                              vs[7] >= -8,  //
                                              vs[7] <= 8,   //
                                              vs[10] >= 0));
}

TEST_P(TestLpDriver, BoundsNoSpace) {
  LpDriver driver{*lp_solver_};
  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         "st\n"
                         "bounds\n"
                         "x1 <= 1\n"
                         "x2 < 2\n"
                         "x3 <= -3\n"
                         "x4 = 4\n"
                         "5 <= x5\n"
                         "6 < x6\n"
                         "7 < x7 < 7\n"
                         "-8 < x8 < 8\n"
                         "-inf < x9 < inf\n"
                         "-infinity < x10\n"
                         "x11 <= inf\n"
                         "x12 free\n"
                         "End"));
  constexpr std::size_t n_variables = 12;
  ASSERT_EQ(lp_solver_->variables().size(), n_variables);
  std::array<Variable, n_variables> vs;
  for (std::size_t i = 0; i < vs.size(); ++i) vs[i] = lp_solver_->variables().at(i);
  std::ranges::sort(vs, {}, &Variable::id);
  fmt::println("{}", vs);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(vs[0] >= 0,   //
                                              vs[0] <= 1,   //
                                              vs[1] >= 0,   //
                                              vs[1] <= 2,   //
                                              vs[2] <= -3,  //
                                              vs[3] == 4,   //
                                              vs[4] >= 5,   //
                                              vs[5] >= 6,   //
                                              vs[6] == 7,   //
                                              vs[7] >= -8,  //
                                              vs[7] <= 8,   //
                                              vs[10] >= 0));
}

TEST_P(TestLpDriver, ConstraintSectionAliases) {
  for (const std::string_view st : {"Subject To", "subject to", "st", "ST", "s.t."}) {
    auto lp_solver = LpSolver::GetInstance(config_);
    LpDriver driver{*lp_solver};

    ASSERT_TRUE(
        driver.ParseString(fmt::format("Minimize\n"
                                       " obj: x1\n"
                                       "{}\n"
                                       " c1: x1 <= 1\n"
                                       "End",
                                       st)));

    EXPECT_EQ(lp_solver->variables().size(), 1u);
    EXPECT_EQ(lp_solver->constraints().size(), 2u);
  }
}

TEST_P(TestLpDriver, BoundsAliases) {
  for (const std::string_view bnd : {"bounds", "Bounds", "BOUNDS"}) {
    auto lp_solver = LpSolver::GetInstance(config_);
    LpDriver driver{*lp_solver};

    ASSERT_TRUE(
        driver.ParseString(fmt::format("Minimize\n"
                                       "{}\n"
                                       " x1 <= 1\n"
                                       "End",
                                       bnd)));

    EXPECT_EQ(lp_solver->variables().size(), 1u);
  }
}

TEST_P(TestLpDriver, CaseInsensitiveAllSections) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("MiNiMiZe\n"
                         " obj: x1\n"
                         "SuBjEcT To\n"
                         " c1: x1 <= 1\n"
                         "BoUnDs\n"
                         " x1 <= 0\n"
                         "EnD"));

  EXPECT_EQ(lp_solver_->variables().size(), 1u);
}

TEST_P(TestLpDriver, ObjectiveDuplicateVariables) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " obj: 4 x1 + 2 x1 - x2 + 3 x2\n"
                         "End"));

  ASSERT_EQ(lp_solver_->variables().size(), 2u);
}

TEST_P(TestLpDriver, ScientificNotationCoefficients) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " obj: 1e2 x1 - 2E-1 x2 + 3.5e+1 x3\n"
                         "End"));

  EXPECT_EQ(lp_solver_->variables().size(), 3u);
}

TEST_P(TestLpDriver, ExplicitMultiplication) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " obj: 2 * x1 - 3 * x2 + 4*x3\n"
                         "End"));

  EXPECT_EQ(lp_solver_->variables().size(), 3u);
}

TEST_P(TestLpDriver, EqualityAliasDoubleEqual) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         "st\n"
                         " c1: x1 + x2 = 5\n"
                         "End"));

  EXPECT_EQ(lp_solver_->variables().size(), 2u);
}

TEST_P(TestLpDriver, ConstraintSenseAliases) {
  for (const std::string_view sense : {"<", "<=", ">", ">=", "=", "=="}) {
    LpDriver driver{*lp_solver_};

    ASSERT_TRUE(
        driver.ParseString(fmt::format("Minimize\n"
                                       "st\n"
                                       " c1: x1 {} 5\n"
                                       "End",
                                       sense)));
  }
}

TEST_P(TestLpDriver, EmptyConstraintSection) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " obj: x1\n"
                         "st\n"
                         "End"));

  EXPECT_EQ(lp_solver_->variables().size(), 1u);
}

TEST_P(TestLpDriver, FreeVariable) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         "bounds\n"
                         " x1 free\n"
                         "End"));

  ASSERT_EQ(lp_solver_->variables().size(), 1u);

  const auto& constraints = lp_solver_->constraints();

  EXPECT_TRUE(constraints.empty());
}

TEST_P(TestLpDriver, InfinityVariants) {
  for (const std::string_view inf : {"inf", "+inf", "infinity", "+infinity"}) {
    auto lp_solver = LpSolver::GetInstance(config_);
    LpDriver driver{*lp_solver};

    ASSERT_TRUE(
        driver.ParseString(fmt::format("Minimize\n"
                                       "bounds\n"
                                       " x1 <= {}\n"
                                       "End",
                                       inf)));
  }
}

TEST_P(TestLpDriver, DuplicateBounds) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         "bounds\n"
                         " 0 <= x1\n"
                         " x1 <= 10\n"
                         " x1 <= 2\n"
                         "End"));

  EXPECT_EQ(lp_solver_->variables().size(), 1u);
}

TEST_P(TestLpDriver, GeneralVariablesSection) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " obj: x1 + x2 + x4\n"
                         "General\n"
                         " x1 x2\n"
                         " x3\n"
                         "End"));

  constexpr std::size_t n_variables = 4;
  ASSERT_EQ(lp_solver_->variables().size(), n_variables);
  std::array<Variable, n_variables> vs;
  for (std::size_t i = 0; i < vs.size(); ++i) vs[i] = lp_solver_->variables().at(i);
  std::ranges::sort(vs, {}, &Variable::name);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(vs[0] >= 0,  //
                                              vs[1] >= 0,  //
                                              vs[2] >= 0,  //
                                              vs[3] >= 0));
}

TEST_P(TestLpDriver, BinaryVariablesSection) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " obj: b1 + b2 + b4\n"
                         "Binary\n"
                         " b1 b2\n"
                         " b3\n"
                         "End"));

  constexpr std::size_t n_variables = 4;
  ASSERT_EQ(lp_solver_->variables().size(), n_variables);
  std::array<Variable, n_variables> vs;
  for (std::size_t i = 0; i < vs.size(); ++i) vs[i] = lp_solver_->variables().at(i);
  std::ranges::sort(vs, {}, &Variable::name);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(vs[0] >= 0,  //
                                              vs[0] <= 1,  //
                                              vs[1] >= 0,  //
                                              vs[1] <= 1,  //
                                              vs[2] >= 0,  //
                                              vs[2] <= 1,  //
                                              vs[3] >= 0));
}

TEST_P(TestLpDriver, GeneralVariablesMultipleLines) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " obj: x1 + x2 + x3\n"
                         "General\n"
                         " x1\n"
                         " x2\n"
                         " x3\n"
                         "End"));

  ASSERT_EQ(lp_solver_->variables().size(), 3u);
}

TEST_P(TestLpDriver, CommentsEverywhere) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("\\ comment 1\n"
                         "Minimize \\ comment obj\n"
                         "\\ comment 2\n"
                         " obj: x1 \\ comment obj\n"
                         "\\ comment 3\n"
                         "st \\ comment st\n"
                         "\\ comment 4\n"
                         " c1: x1 <= 1 \\ comment c1\n"
                         "\\ comment 5\n"
                         "bounds \\comments bounds\n"
                         "\\ comment 6\n"
                         " x1 <= 4 \\comment bound\n"
                         "End"));

  ASSERT_EQ(lp_solver_->variables().size(), 1u);
  const Variable& v = lp_solver_->variables().at(0);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints, ::testing::UnorderedElementsAre(v >= 0, v <= 1, v <= 4));
}

TEST_P(TestLpDriver, GurobiStyle) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("\\ Model SympyToGurobi\n"
                         "\\ LP format - for model browsing. Use MPS format to capture full model detail.\n"
                         "\\ Signature: 0x39a33347cbbbdc3e\n"
                         "Maximize\n"
                         "4 x0 + 1e-15 x1 + 1e-15 x2\n"
                         "Subject To\n"
                         "c_1: - 0.1873813145857246 x0 + 0.9822872507286887 x1 <= 1\n"
                         "Bounds\n"
                         "-infinity <= x0 <= 1\n"
                         "x1 free\n"
                         "End\n"));

  constexpr std::size_t n_variables = 3;
  ASSERT_EQ(lp_solver_->variables().size(), n_variables);
  std::array<Variable, n_variables> vs;
  for (std::size_t i = 0; i < vs.size(); ++i) vs[i] = lp_solver_->variables().at(i);
  std::ranges::sort(vs, {}, &Variable::name);
  const std::vector<Formula> constraints = lp_solver_->constraints();
  EXPECT_THAT(constraints,
              ::testing::UnorderedElementsAre(delpi::gmp::StringToMpq("-0.1873813145857246") * vs[0] +
                                                      delpi::gmp::StringToMpq("0.9822872507286887") * vs[1] <=
                                                  1,
                                              vs[0] <= 1,  //
                                              vs[2] >= 0));
}

TEST_P(TestLpDriver, MissingEndKeyword) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " obj: x1\n"));

  EXPECT_EQ(lp_solver_->variables().size(), 1u);
}

TEST_P(TestLpDriver, VariableNamesWithSymbols) {
  LpDriver driver{*lp_solver_};

  ASSERT_TRUE(
      driver.ParseString("Minimize\n"
                         " obj: x_1 + abc.def + var/name + var@id\n"
                         "End"));

  EXPECT_EQ(lp_solver_->variables().size(), 4u);
  std::vector<std::string> var_names;
  std::ranges::transform(lp_solver_->variables(), std::back_inserter(var_names), &Variable::name);
  EXPECT_THAT(var_names, ::testing::UnorderedElementsAre("x_1", "abc.def", "var/name", "var@id"));
}