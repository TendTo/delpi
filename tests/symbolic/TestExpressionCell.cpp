/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "delpi/symbolic/ExpressionCell.h"

using delpi::Expression;
using delpi::ExpressionCell;
using delpi::intrusive_ptr;
using delpi::LinearMonomial;
using delpi::Variable;

class TestExpressionCell : public ::testing::Test {
 protected:
  const Variable x_{"x"};
  const Variable y_{"y"};
  const Variable z_{"z"};

  intrusive_ptr<ExpressionCell> e1_{ExpressionCell::New()};
  intrusive_ptr<ExpressionCell> e2_{ExpressionCell::New()};
};

TEST_F(TestExpressionCell, ProtectedConstructor) {
  static_assert(!std::is_default_constructible_v<ExpressionCell>,
                "ExpressionCell should not be default_constructible.");
  static_assert(!std::is_constructible_v<ExpressionCell>, "ExpressionCell should not be constructible.");
}

TEST_F(TestExpressionCell, NewInstanceDefault) {
  const intrusive_ptr<ExpressionCell> cell{ExpressionCell::New()};
  EXPECT_EQ(cell->use_count(), 1u);
  EXPECT_TRUE(cell->addends().empty());
  EXPECT_EQ(cell->hash(), 0u);
  EXPECT_TRUE(cell->equal_to(*cell));
  EXPECT_FALSE(cell->less(*cell));
  EXPECT_TRUE(cell->variables().empty());
  EXPECT_EQ(cell->Evaluate(), 0);
  EXPECT_TRUE(cell->Substitute({}).addends().empty());
}

TEST_F(TestExpressionCell, NewInstanceVar) {
  const intrusive_ptr<ExpressionCell> cell{ExpressionCell::New(x_)};
  EXPECT_EQ(cell->use_count(), 1u);
  ASSERT_EQ(cell->addends().size(), 1u);
  EXPECT_TRUE(cell->addends().cbegin()->first.equal_to(x_));
  EXPECT_EQ(cell->addends().cbegin()->second, mpq_class{1});
  EXPECT_GT(cell->hash(), 0);
  EXPECT_TRUE(cell->equal_to(*cell));
  EXPECT_FALSE(cell->less(*cell));
  ASSERT_EQ(cell->variables().size(), 1u);
  EXPECT_TRUE(cell->variables().front().equal_to(x_));
}

TEST_F(TestExpressionCell, Copy) {
  intrusive_ptr<ExpressionCell> cell{ExpressionCell::New(x_)};
  {
    intrusive_ptr<ExpressionCell> cell_copy{ExpressionCell::Copy(*cell)};
    cell_copy->Add(y_, 1);
    EXPECT_EQ(cell_copy->use_count(), 1u);
    EXPECT_EQ(cell->use_count(), 1u);
    EXPECT_EQ(cell_copy->variables().size(), 2u);
    EXPECT_EQ(cell->variables().size(), 1u);
  }
  EXPECT_EQ(cell->use_count(), 1u);
}

TEST_F(TestExpressionCell, NewInstanceLinearMonomial) {
  const intrusive_ptr<ExpressionCell> cell{ExpressionCell::New(LinearMonomial{x_, 2})};
  EXPECT_EQ(cell->use_count(), 1u);
  ASSERT_EQ(cell->addends().size(), 1u);
  EXPECT_TRUE(cell->addends().cbegin()->first.equal_to(x_));
  EXPECT_EQ(cell->addends().cbegin()->second, mpq_class{2});
  EXPECT_GT(cell->hash(), 0);
  EXPECT_TRUE(cell->equal_to(*cell));
  EXPECT_FALSE(cell->less(*cell));
  ASSERT_EQ(cell->variables().size(), 1u);
  EXPECT_TRUE(cell->variables().front().equal_to(x_));
}

TEST_F(TestExpressionCell, NewInstanceAddends) {
  const intrusive_ptr<ExpressionCell> cell = ExpressionCell::New({{x_, 1}, {y_, 2}, {z_, 6}});
  EXPECT_EQ(cell->use_count(), 1u);
  ASSERT_EQ(cell->addends().size(), 3u);
  EXPECT_EQ(cell->addends().at(x_), 1);
  EXPECT_EQ(cell->addends().at(y_), 2);
  EXPECT_EQ(cell->addends().at(z_), 6);
  EXPECT_GT(cell->hash(), 0);
  EXPECT_TRUE(cell->equal_to(*cell));
  EXPECT_FALSE(cell->less(*cell));
  ASSERT_EQ(cell->variables().size(), 3u);
}

TEST_F(TestExpressionCell, CopyReferenceCount) {
  const intrusive_ptr<ExpressionCell> cell{ExpressionCell::New(LinearMonomial{x_, 2})};
  EXPECT_EQ(cell->use_count(), 1u);
  {
    [[maybe_unused]] const intrusive_ptr<ExpressionCell> cell_constructor{cell};
    EXPECT_EQ(cell->use_count(), 2u);
    [[maybe_unused]] const intrusive_ptr<ExpressionCell> cell_copy = cell;
    EXPECT_EQ(cell->use_count(), 3u);
  }
  EXPECT_EQ(cell->use_count(), 1u);
}

TEST_F(TestExpressionCell, MoveReferenceCount) {
  intrusive_ptr<ExpressionCell> cell{ExpressionCell::New(LinearMonomial{x_, 2})};
  EXPECT_EQ(cell->use_count(), 1u);
  {
    intrusive_ptr<ExpressionCell> cell_constructor{std::move(cell)};
    EXPECT_EQ(cell_constructor->use_count(), 1u);
    EXPECT_EQ(cell.get(), nullptr);
    [[maybe_unused]] const intrusive_ptr<ExpressionCell> cell_copy = std::move(cell_constructor);
    EXPECT_EQ(cell_copy->use_count(), 1u);
    EXPECT_EQ(cell_constructor.get(), nullptr);
  }
}

TEST_F(TestExpressionCell, Hash) {
  EXPECT_EQ(e1_->hash(), e2_->hash());
  const auto c1 = ExpressionCell::Copy(*e1_);
  EXPECT_EQ(c1->hash(), e1_->hash());
  e1_->Add(x_, 4);
  EXPECT_NE(c1->hash(), e1_->hash());
  const auto c2 = ExpressionCell::Copy(*e1_);
  EXPECT_EQ(c2->hash(), e1_->hash());
  e1_->Multiply(2);
  EXPECT_NE(c2->hash(), e1_->hash());
  const auto c3 = ExpressionCell::Copy(*e1_);
  EXPECT_EQ(c3->hash(), e1_->hash());
  e1_->Divide(2);
  EXPECT_NE(c3->hash(), e1_->hash());

  EXPECT_EQ(c2->hash(), e1_->hash());
  e1_->Add(x_, -4);
  EXPECT_EQ(e1_->hash(), e2_->hash());
}

TEST_F(TestExpressionCell, Add) {
  e1_->Add(x_, 1);
  EXPECT_EQ(e1_->variables().size(), 1u);
  ASSERT_EQ(e1_->addends().size(), 1u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{1});

  e1_->Add(x_, 4);
  EXPECT_EQ(e1_->variables().size(), 1u);
  ASSERT_EQ(e1_->addends().size(), 1u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{5});

  e1_->Add(x_, -6);
  EXPECT_EQ(e1_->variables().size(), 1u);
  ASSERT_EQ(e1_->addends().size(), 1u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{-1});

  e1_->Add(y_, -7);
  EXPECT_EQ(e1_->variables().size(), 2u);
  ASSERT_EQ(e1_->addends().size(), 2u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{-1});
  EXPECT_EQ(e1_->addends().at(y_), mpq_class{-7});

  e1_->Add(y_, 7);
  EXPECT_EQ(e1_->variables().size(), 1u);
  ASSERT_EQ(e1_->addends().size(), 1u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{-1});
}

TEST_F(TestExpressionCell, Multiply) {
  e1_->Add(x_, 7);
  e1_->Add(y_, 12);
  e1_->Multiply(2);
  EXPECT_EQ(e1_->variables().size(), 2u);
  ASSERT_EQ(e1_->addends().size(), 2u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{14});
  EXPECT_EQ(e1_->addends().at(y_), mpq_class{24});

  e1_->Multiply(1);
  EXPECT_EQ(e1_->variables().size(), 2u);
  ASSERT_EQ(e1_->addends().size(), 2u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{14});
  EXPECT_EQ(e1_->addends().at(y_), mpq_class{24});

  e1_->Multiply(-1);
  EXPECT_EQ(e1_->variables().size(), 2u);
  ASSERT_EQ(e1_->addends().size(), 2u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{-14});
  EXPECT_EQ(e1_->addends().at(y_), mpq_class{-24});

  e1_->Add(x_, 1);
  e1_->Add(y_, 1);
  e1_->Add(z_, 1);
  EXPECT_EQ(e1_->variables().size(), 3u);
  ASSERT_EQ(e1_->addends().size(), 3u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{-13});
  EXPECT_EQ(e1_->addends().at(y_), mpq_class{-23});
  EXPECT_EQ(e1_->addends().at(z_), mpq_class{1});

  e1_->Multiply(0);
  EXPECT_EQ(e1_->variables().size(), 0u);
  ASSERT_EQ(e1_->addends().size(), 0u);
}

TEST_F(TestExpressionCell, Divide) {
  e1_->Add(x_, 18);
  e1_->Add(y_, 12);
  e1_->Divide(2);
  EXPECT_EQ(e1_->variables().size(), 2u);
  ASSERT_EQ(e1_->addends().size(), 2u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{9});
  EXPECT_EQ(e1_->addends().at(y_), mpq_class{6});

  e1_->Divide(1);
  EXPECT_EQ(e1_->variables().size(), 2u);
  ASSERT_EQ(e1_->addends().size(), 2u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{9});
  EXPECT_EQ(e1_->addends().at(y_), mpq_class{6});

  e1_->Divide(-1);
  EXPECT_EQ(e1_->variables().size(), 2u);
  ASSERT_EQ(e1_->addends().size(), 2u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{-9});
  EXPECT_EQ(e1_->addends().at(y_), mpq_class{-6});

  e1_->Add(x_, 1);
  e1_->Add(y_, 1);
  e1_->Add(z_, 1);
  EXPECT_EQ(e1_->variables().size(), 3u);
  ASSERT_EQ(e1_->addends().size(), 3u);
  EXPECT_EQ(e1_->addends().at(x_), mpq_class{-8});
  EXPECT_EQ(e1_->addends().at(y_), mpq_class{-5});
  EXPECT_EQ(e1_->addends().at(z_), mpq_class{1});

  EXPECT_ANY_THROW(e1_->Divide(0));
}

TEST_F(TestExpressionCell, NoThrowComparators) {
  static_assert(std::is_nothrow_invocable_r_v<bool, decltype(&ExpressionCell::equal_to), const ExpressionCell&,
                                              const ExpressionCell&>,
                "Method equal_to should be is_nothrow_invocable.");
  static_assert(std::is_nothrow_invocable_r_v<bool, decltype(&ExpressionCell::less), const ExpressionCell&,
                                              const ExpressionCell&>,
                "Method less should be is_nothrow_invocable.");
  static_assert(std::is_nothrow_invocable_r_v<bool, decltype(&ExpressionCell::hash), const ExpressionCell&>,
                "Method hash should be is_nothrow_invocable");
}

#ifndef DELPI_THREAD_SAFE
TEST_F(TestExpressionCell, NoThrowMoveSintax) {
  static_assert(std::is_nothrow_move_constructible_v<ExpressionCell>,
                "ExpressionCell should be nothrow_move_constructible.");
  static_assert(std::is_nothrow_move_assignable_v<ExpressionCell>, "ExpressionCell should be nothrow_move_assignable.");
}
#endif
