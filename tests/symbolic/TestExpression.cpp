/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/symbolic/Expression.h"

using delpi::Expression;
using delpi::Variable;

class TestExpression : public ::testing::Test {
 protected:
  const Variable x_{"x"};
  const Variable y_{"y"};
  const Variable z_{"z"};

  Expression e1_{};
  Expression e2_{};
};

TEST_F(TestExpression, DefaultConstructor) {
  const Expression e;
  EXPECT_TRUE(e.addends().empty());
  EXPECT_EQ(e.hash(), 0u);
  EXPECT_TRUE(e.equal_to(e));
  EXPECT_FALSE(e.less(e));
  EXPECT_TRUE(e.variables().empty());
  EXPECT_EQ(e.Evaluate(), 0);
  EXPECT_TRUE(e.Substitute({}).addends().empty());
}

TEST_F(TestExpression, VarConstructor) {
  const Expression e{x_};
  ASSERT_EQ(e.addends().size(), 1u);
  EXPECT_EQ(e.addends().at(x_), mpq_class{1});
  EXPECT_GT(e.hash(), 0);
  EXPECT_TRUE(e.equal_to(e));
  EXPECT_FALSE(e.less(e));
  ASSERT_EQ(e.variables().size(), 1u);
  EXPECT_TRUE(e.variables().front().equal_to(x_));
}

TEST_F(TestExpression, LinearMonomialConstructor) {
  const Expression e{Expression::Addend{x_, 2}};
  ASSERT_EQ(e.addends().size(), 1u);
  EXPECT_TRUE(e.addends().cbegin()->first.equal_to(x_));
  EXPECT_EQ(e.addends().cbegin()->second, mpq_class{2});
  EXPECT_GT(e.hash(), 0);
  EXPECT_TRUE(e.equal_to(e));
  EXPECT_FALSE(e.less(e));
  ASSERT_EQ(e.variables().size(), 1u);
  EXPECT_TRUE(e.variables().front().equal_to(x_));
}

TEST_F(TestExpression, AddendsConstructor) {
  const Expression e{Expression::Addends{{x_, 1}, {y_, 2}, {z_, 6}}};
  ASSERT_EQ(e.addends().size(), 3u);
  EXPECT_EQ(e.addends().at(x_), 1);
  EXPECT_EQ(e.addends().at(y_), 2);
  EXPECT_EQ(e.addends().at(z_), 6);
  EXPECT_GT(e.hash(), 0);
  EXPECT_TRUE(e.equal_to(e));
  EXPECT_FALSE(e.less(e));
  ASSERT_EQ(e.variables().size(), 3u);
}

TEST_F(TestExpression, Copy) {
  const Expression e;
  {
    Expression e_copy{e};
    EXPECT_EQ(e.use_count(), 2u);
    EXPECT_EQ(e_copy.use_count(), 2u);

    e_copy.Add(x_, 1);
    EXPECT_EQ(e.use_count(), 1u);
    EXPECT_EQ(e_copy.use_count(), 1u);
    EXPECT_EQ(e_copy.variables().size(), 1u);
    EXPECT_EQ(e.variables().size(), 0u);
  }
}

TEST_F(TestExpression, CopyReferenceCount) {
  const Expression e{Expression::Addend{x_, 2}};
  EXPECT_EQ(e.use_count(), 1u);
  {
    const Expression e_constructor{e};
    EXPECT_EQ(e.use_count(), 2u);
    const Expression e_copy = e;
    EXPECT_EQ(e.use_count(), 3u);
  }
  EXPECT_EQ(e.use_count(), 1u);
}

TEST_F(TestExpression, MoveReferenceCount) {
  Expression e{Expression::Addend{x_, 2}};
  EXPECT_EQ(e.use_count(), 1u);
  {
    Expression e_constructor{std::move(e)};
    EXPECT_EQ(e_constructor.use_count(), 1u);
    Expression e_copy = std::move(e_constructor);
    EXPECT_EQ(e_copy.use_count(), 1u);
  }
}

TEST_F(TestExpression, Hash) {
  EXPECT_EQ(e1_.hash(), e2_.hash());
  const auto c1 = e1_;
  EXPECT_EQ(c1.hash(), e1_.hash());
  e1_.Add(x_, 4);
  EXPECT_NE(c1.hash(), e1_.hash());
  const auto c2 = e1_;
  EXPECT_EQ(c2.hash(), e1_.hash());
  e1_ *= 2;
  EXPECT_NE(c2.hash(), e1_.hash());
  const auto c3 = e1_;
  EXPECT_EQ(c3.hash(), e1_.hash());
  e1_ /= 2;
  EXPECT_NE(c3.hash(), e1_.hash());

  EXPECT_EQ(c2.hash(), e1_.hash());
  e1_.Add(x_, -4);
  EXPECT_EQ(e1_.hash(), e2_.hash());
}

TEST_F(TestExpression, Add) {
  e1_.Add(x_, 1);
  EXPECT_EQ(e1_.variables().size(), 1u);
  ASSERT_EQ(e1_.addends().size(), 1u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{1});

  e1_.Add(x_, 4);
  EXPECT_EQ(e1_.variables().size(), 1u);
  ASSERT_EQ(e1_.addends().size(), 1u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{5});

  e1_.Add(x_, -6);
  EXPECT_EQ(e1_.variables().size(), 1u);
  ASSERT_EQ(e1_.addends().size(), 1u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{-1});

  e1_.Add(y_, -7);
  EXPECT_EQ(e1_.variables().size(), 2u);
  ASSERT_EQ(e1_.addends().size(), 2u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{-1});
  EXPECT_EQ(e1_.addends().at(y_), mpq_class{-7});

  e1_.Add(y_, 7);
  EXPECT_EQ(e1_.variables().size(), 1u);
  ASSERT_EQ(e1_.addends().size(), 1u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{-1});
}

TEST_F(TestExpression, Multiply) {
  e1_.Add(x_, 7);
  e1_.Add(y_, 12);
  e1_ *= 2;
  EXPECT_EQ(e1_.variables().size(), 2u);
  ASSERT_EQ(e1_.addends().size(), 2u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{14});
  EXPECT_EQ(e1_.addends().at(y_), mpq_class{24});

  e1_ *= 1;
  EXPECT_EQ(e1_.variables().size(), 2u);
  ASSERT_EQ(e1_.addends().size(), 2u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{14});
  EXPECT_EQ(e1_.addends().at(y_), mpq_class{24});

  e1_ *= -1;
  EXPECT_EQ(e1_.variables().size(), 2u);
  ASSERT_EQ(e1_.addends().size(), 2u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{-14});
  EXPECT_EQ(e1_.addends().at(y_), mpq_class{-24});

  e1_.Add(x_, 1);
  e1_.Add(y_, 1);
  e1_.Add(z_, 1);
  EXPECT_EQ(e1_.variables().size(), 3u);
  ASSERT_EQ(e1_.addends().size(), 3u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{-13});
  EXPECT_EQ(e1_.addends().at(y_), mpq_class{-23});
  EXPECT_EQ(e1_.addends().at(z_), mpq_class{1});

  e1_ *= 0;
  EXPECT_EQ(e1_.variables().size(), 0u);
  ASSERT_EQ(e1_.addends().size(), 0u);
}

TEST_F(TestExpression, Divide) {
  e1_.Add(x_, 18);
  e1_.Add(y_, 12);
  e1_ /= 2;
  EXPECT_EQ(e1_.variables().size(), 2u);
  ASSERT_EQ(e1_.addends().size(), 2u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{9});
  EXPECT_EQ(e1_.addends().at(y_), mpq_class{6});

  e1_ /= 1;
  EXPECT_EQ(e1_.variables().size(), 2u);
  ASSERT_EQ(e1_.addends().size(), 2u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{9});
  EXPECT_EQ(e1_.addends().at(y_), mpq_class{6});

  e1_ /= -1;
  EXPECT_EQ(e1_.variables().size(), 2u);
  ASSERT_EQ(e1_.addends().size(), 2u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{-9});
  EXPECT_EQ(e1_.addends().at(y_), mpq_class{-6});

  e1_.Add(x_, 1);
  e1_.Add(y_, 1);
  e1_.Add(z_, 1);
  EXPECT_EQ(e1_.variables().size(), 3u);
  ASSERT_EQ(e1_.addends().size(), 3u);
  EXPECT_EQ(e1_.addends().at(x_), mpq_class{-8});
  EXPECT_EQ(e1_.addends().at(y_), mpq_class{-5});
  EXPECT_EQ(e1_.addends().at(z_), mpq_class{1});

  EXPECT_ANY_THROW(e1_ /= (0));
}

TEST_F(TestExpression, NegativeVariable) {
  const Expression e1{-x_};
  ASSERT_EQ(e1.addends().size(), 1u);
  EXPECT_EQ(e1.addends().at(x_), mpq_class{-1});
}

TEST_F(TestExpression, ComplexExpressions) {
  const Expression e1{x_ + 2 * y_ + 3 * z_};
  EXPECT_EQ(e1.variables().size(), 3u);
  EXPECT_EQ(e1.Evaluate(Expression::Environment{{x_, 1}, {y_, 2}, {z_, 3}}), 14);

  const Expression e2{x_ + 2 * y_ + 3 * z_ + z_ + 4 * y_ * 5 + 6 * x_ * 7 - 8 * x_ - z_ * 4 + x_ / 2};
  EXPECT_EQ(e2.variables().size(), 2u);
  EXPECT_EQ(e2.Evaluate(Expression::Environment{{x_, 1}, {y_, 2}}), 79 + mpq_class(1, 2));
}

TEST_F(TestExpression, SumExpression) {
  const Expression e1{x_ + 2 * y_ + 3 * z_ + z_ + 4 * y_ * 5};
  const Expression e2{6 * x_ * 7 - 8 * x_ - z_ * 4 + x_ / 2};
  EXPECT_EQ((e1 + e2).variables().size(), 2u);
  EXPECT_EQ((e1 + e2).Evaluate(Expression::Environment{{x_, 1}, {y_, 2}}), 79 + mpq_class(1, 2));
}

TEST_F(TestExpression, Subtractxpression) {
  const Expression e1{x_ + 2 * y_ + 3 * z_ + z_ + 4 * y_ * 5 - 8 * z_};
  const Expression e2{6 * x_ * 7 - 8 * x_ - z_ * 4 + x_ / 2};
  EXPECT_EQ((e1 - e2).variables().size(), 2u);
  EXPECT_EQ((e1 - e2).Evaluate(Expression::Environment{{x_, 1}, {y_, 2}}), -mpq_class(67, 2) + 44);
}

#ifndef DELPI_THREAD_SAFE
TEST_F(TestExpression, NoThrowMoveSintax) {
  static_assert(std::is_nothrow_move_constructible_v<Expression>, "Expression should be nothrow_move_constructible.");
  static_assert(std::is_nothrow_move_assignable_v<Expression>, "Expression should be nothrow_move_assignable.");
}
#endif

TEST_F(TestExpression, NoThrowComparators) {
  static_assert(
      std::is_nothrow_invocable_r_v<bool, decltype(&Expression::equal_to), const Expression&, const Expression&>,
      "Method equal_to should be is_nothrow_invocable.");
  static_assert(std::is_nothrow_invocable_r_v<bool, decltype(&Expression::less), const Expression&, const Expression&>,
                "Method less should be is_nothrow_invocable.");
  static_assert(std::is_nothrow_invocable_r_v<bool, decltype(&Expression::hash), const Expression&>,
                "Method hash should be is_nothrow_invocable");
}