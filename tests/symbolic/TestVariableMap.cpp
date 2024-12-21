/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/symbolic/VariableMap.h"
#include "delpi/util/exception.h"

using delpi::DelpiOutOfRangeException;
using delpi::Variable;
using VariableMap = delpi::VariableMap<double>;
using ItemVector = std::vector<std::pair<const Variable, double>>;

class TestVariableMap : public ::testing::Test {
 protected:
  const Variable x_{"x"};
  const Variable y_{"y"};
  const Variable z_{"z"};
  const Variable w_{"w"};
  const ItemVector items_{{x_, 1}, {y_, 2}, {z_, 3}};
  VariableMap vs_;

  static bool CompareVariables(const Variable& var1, const Variable& var2) { return var1.equal_to(var2); }
};

TEST_F(TestVariableMap, DefaultConstructor) {
  const VariableMap vs;
  EXPECT_TRUE(vs.empty());
  EXPECT_EQ(vs.size(), 0u);
}

TEST_F(TestVariableMap, RangeConstructor) {
  const VariableMap vs{ItemVector{{x_, 1}, {y_, 2}, {z_, 3}}};
  EXPECT_EQ(vs.size(), 3u);
  EXPECT_EQ(vs.capacity(), 3u);
  EXPECT_TRUE(vs.Contains(x_));
  EXPECT_TRUE(vs.Contains(y_));
  EXPECT_TRUE(vs.Contains(z_));
}

TEST_F(TestVariableMap, AddSingleVariable) {
  EXPECT_TRUE(vs_.Insert(x_, 1));
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_EQ(vs_.At(x_), 1);
  EXPECT_EQ(vs_.size(), 1u);
  EXPECT_EQ(vs_.capacity(), 1u);
  EXPECT_FALSE(vs_.Insert(x_, 2));
  EXPECT_EQ(vs_.size(), 1u);
  EXPECT_EQ(vs_.capacity(), 1u);
  EXPECT_EQ(vs_.At(x_), 2);
}

TEST_F(TestVariableMap, AddMultipleSteps) {
  EXPECT_TRUE(vs_.Insert(y_, 2));
  EXPECT_EQ(vs_.capacity(), 1u);
  EXPECT_TRUE(vs_.Insert(z_, 2));
  EXPECT_EQ(vs_.capacity(), 2u);
  EXPECT_TRUE(vs_.Insert(x_, 3));
  EXPECT_EQ(vs_.capacity(), 3u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_TRUE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
}

TEST_F(TestVariableMap, AddSparse) {
  EXPECT_TRUE(vs_.Insert(x_, 1));
  EXPECT_EQ(vs_.capacity(), 1u);
  EXPECT_TRUE(vs_.Insert(z_, 2));
  EXPECT_EQ(vs_.capacity(), 3u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
}

TEST_F(TestVariableMap, AddRangeSparse) {
  EXPECT_TRUE(vs_.Insert(y_, 2));
  EXPECT_EQ(vs_.capacity(), 1u);
  vs_.Insert(ItemVector{{z_, 3}, {x_, 1}});
  EXPECT_EQ(vs_.capacity(), 3u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_TRUE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
}

TEST_F(TestVariableMap, AddBigRangeSparseLeft) {
  EXPECT_TRUE(vs_.Insert(x_, 1));
  EXPECT_EQ(vs_.capacity(), 1u);
  vs_.Insert(ItemVector{{w_, 4}});
  EXPECT_EQ(vs_.capacity(), 4u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
  EXPECT_FALSE(vs_.Contains(z_));
  EXPECT_TRUE(vs_.Contains(w_));
}

TEST_F(TestVariableMap, AddBigRangeSparseRight) {
  EXPECT_TRUE(vs_.Insert(w_, 4));
  EXPECT_EQ(vs_.capacity(), 1u);
  vs_.Insert(ItemVector{{x_, 1}});
  EXPECT_EQ(vs_.capacity(), 4u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
  EXPECT_FALSE(vs_.Contains(z_));
  EXPECT_TRUE(vs_.Contains(w_));
}

TEST_F(TestVariableMap, AddBigRangeSparseMiddleLeft) {
  EXPECT_TRUE(vs_.Insert(y_, 2));
  EXPECT_EQ(vs_.capacity(), 1u);
  vs_.Insert(ItemVector{{w_, 4}, {x_, 1}});
  EXPECT_EQ(vs_.capacity(), 4u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_TRUE(vs_.Contains(y_));
  EXPECT_FALSE(vs_.Contains(z_));
  EXPECT_TRUE(vs_.Contains(w_));
}

TEST_F(TestVariableMap, AddBigRangeSparseMiddleRight) {
  EXPECT_TRUE(vs_.Insert(z_, 3));
  EXPECT_EQ(vs_.capacity(), 1u);
  vs_.Insert(ItemVector{{w_, 4}, {x_, 1}});
  EXPECT_EQ(vs_.capacity(), 4u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
  EXPECT_TRUE(vs_.Contains(w_));
}

TEST_F(TestVariableMap, AddRangeVariables) {
  vs_.Insert(items_);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_TRUE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
  EXPECT_EQ(vs_.size(), 3u);
}

TEST_F(TestVariableMap, RemoveSingleVariable) {
  vs_.Insert(x_, 1);
  EXPECT_TRUE(vs_.Remove(x_));
  EXPECT_FALSE(vs_.Contains(x_));
  EXPECT_EQ(vs_.size(), 0u);
  EXPECT_FALSE(vs_.Remove(x_));
}

TEST_F(TestVariableMap, RemoveRangeVariables) {
  vs_.Insert(items_);
  vs_.Remove(std::vector{x_, y_});
  EXPECT_FALSE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
  EXPECT_EQ(vs_.size(), 1u);
}

TEST_F(TestVariableMap, ContainsVariable) {
  vs_.Insert(x_, 1);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
}

TEST_F(TestVariableMap, GetVariable) {
  vs_.Insert(x_, 1);
  EXPECT_EQ(vs_.At(x_), 1);
  EXPECT_THROW([[maybe_unused]] auto _ = vs_.At(y_), DelpiOutOfRangeException);
}

TEST_F(TestVariableMap, ClearVariables) {
  vs_.Insert(items_);
  vs_.Clear();
  EXPECT_TRUE(vs_.empty());
  EXPECT_EQ(vs_.size(), 0u);
}

TEST_F(TestVariableMap, MinMaxId) {
  vs_.Insert(items_);
  EXPECT_EQ(vs_.min_id(), x_.id());
  EXPECT_EQ(vs_.max_id(), z_.id() + 1);
}
