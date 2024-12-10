/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/symbolic/Formula.h"

using delpi::Expression;
using delpi::Formula;
using delpi::FormulaKind;
using delpi::Variable;

class TestFormula : public ::testing::Test {
 protected:
  const Variable x_{"x"};
  const Variable y_{"y"};
  const Variable z_{"z"};

  Expression e1_{2 * x_ + 3 * y_ + 4 * z_};
  Expression e2_{x_ - y_ + 2 * z_};
};

TEST_F(TestFormula, Constructor) {
  const Formula f{e1_, FormulaKind::Eq, 2};
  EXPECT_TRUE(f.expression().equal_to(e1_));
  EXPECT_EQ(f.kind(), FormulaKind::Eq);
  EXPECT_EQ(f.rhs(), 2);
}

TEST_F(TestFormula, Equality) {
  const Formula f1{e1_, FormulaKind::Eq, 2};
  const Formula f2{e1_, FormulaKind::Eq, 2};
  const Formula f3{e2_, FormulaKind::Neq, 3};

  EXPECT_TRUE(f1.equal_to(f2));
  EXPECT_FALSE(f1.equal_to(f3));
}

TEST_F(TestFormula, Less) {
  const Formula f1{e1_, FormulaKind::Eq, 2};
  const Formula f2{e2_, FormulaKind::Neq, 3};

  EXPECT_TRUE(f1.less(f2) || f2.less(f1));
}

TEST_F(TestFormula, Hash) {
  const Formula f1{e1_, FormulaKind::Eq, 2};
  const Formula f2{e1_, FormulaKind::Eq, 2};
  const Formula f3{e2_, FormulaKind::Neq, 3};

  EXPECT_EQ(f1.hash(), f2.hash());
  EXPECT_NE(f1.hash(), f3.hash());
}

TEST_F(TestFormula, OperatorEquals) {
  const Formula f = (x_ == y_);
  EXPECT_EQ(f.kind(), FormulaKind::Eq);
}

TEST_F(TestFormula, OperatorNotEquals) {
  const Formula f = (x_ != y_);
  EXPECT_EQ(f.kind(), FormulaKind::Neq);
}

TEST_F(TestFormula, OperatorLessThan) {
  const Formula f = (x_ < y_);
  EXPECT_EQ(f.kind(), FormulaKind::Lt);
}

TEST_F(TestFormula, OperatorLessThanOrEqual) {
  const Formula f = (x_ <= y_);
  EXPECT_EQ(f.kind(), FormulaKind::Leq);
}

TEST_F(TestFormula, OperatorGreaterThan) {
  const Formula f = (x_ > y_);
  EXPECT_EQ(f.kind(), FormulaKind::Gt);
}

TEST_F(TestFormula, OperatorGreaterThanOrEqual) {
  const Formula f = (x_ >= y_);
  EXPECT_EQ(f.kind(), FormulaKind::Geq);
}

TEST_F(TestFormula, Ostream) {
  const Formula f{e1_, FormulaKind::Eq, 2};
  EXPECT_EQ((std::stringstream{} << f).str(), "((2 * x + 3 * y + 4 * z) = 2)");
}

#ifndef DELPI_THREAD_SAFE
TEST_F(TestFormula, NoThrowMoveSintax) {
  static_assert(std::is_nothrow_move_constructible_v<Formula>, "Formula should be nothrow_move_constructible.");
  static_assert(std::is_nothrow_move_assignable_v<Formula>, "Formula should be nothrow_move_assignable.");
}
#endif

TEST_F(TestFormula, NoThrowComparators) {
  static_assert(std::is_nothrow_invocable_r_v<bool, decltype(&Formula::equal_to), const Formula&, const Formula&>,
                "Method equal_to should be is_nothrow_invocable.");
  static_assert(std::is_nothrow_invocable_r_v<bool, decltype(&Formula::less), const Formula&, const Formula&>,
                "Method less should be is_nothrow_invocable.");
  static_assert(std::is_nothrow_invocable_r_v<bool, decltype(&Formula::hash), const Formula&>,
                "Method hash should be is_nothrow_invocable");
}