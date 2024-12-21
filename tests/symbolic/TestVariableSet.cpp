/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include <set>

#include "delpi/symbolic/VariableSet.h"
#include "delpi/util/exception.h"

using delpi::DelpiOutOfRangeException;
using delpi::Variable;
using delpi::VariableSet;

class TestVariableSet : public ::testing::Test {
 protected:
  const Variable x_{"x"};
  const Variable y_{"y"};
  const Variable z_{"z"};
  const Variable w_{"w"};
  VariableSet vs_;

  static bool CompareVariables(const Variable& var1, const Variable& var2) { return var1.equal_to(var2); }
};

TEST_F(TestVariableSet, DefaultConstructor) {
  const VariableSet vs;
  EXPECT_TRUE(vs.empty());
  EXPECT_EQ(vs.size(), 0u);
}

TEST_F(TestVariableSet, RangeConstructor) {
  const VariableSet vs{std::vector{x_, y_, z_}};
  EXPECT_EQ(vs.size(), 3u);
  EXPECT_EQ(vs.capacity(), 3u);
  EXPECT_TRUE(vs.Contains(x_));
  EXPECT_TRUE(vs.Contains(y_));
  EXPECT_TRUE(vs.Contains(z_));
}

TEST_F(TestVariableSet, SequenceConstructor) {
  const VariableSet vs{x_, y_, z_};
  EXPECT_EQ(vs.size(), 3u);
  EXPECT_EQ(vs.capacity(), 3u);
  EXPECT_TRUE(vs.Contains(x_));
  EXPECT_TRUE(vs.Contains(y_));
  EXPECT_TRUE(vs.Contains(z_));
}

TEST_F(TestVariableSet, AddSingleVariable) {
  EXPECT_TRUE(vs_.Insert(x_));
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_EQ(vs_.size(), 1u);
  EXPECT_EQ(vs_.capacity(), 1u);
  EXPECT_FALSE(vs_.Insert(x_));
  EXPECT_EQ(vs_.size(), 1u);
  EXPECT_EQ(vs_.capacity(), 1u);
}

TEST_F(TestVariableSet, AddMultipleSteps) {
  EXPECT_TRUE(vs_.Insert(y_));
  EXPECT_EQ(vs_.capacity(), 1u);
  EXPECT_TRUE(vs_.Insert(z_));
  EXPECT_EQ(vs_.capacity(), 2u);
  EXPECT_TRUE(vs_.Insert(x_));
  EXPECT_EQ(vs_.capacity(), 3u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_TRUE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
}

TEST_F(TestVariableSet, AddSparse) {
  EXPECT_TRUE(vs_.Insert(x_));
  EXPECT_EQ(vs_.capacity(), 1u);
  EXPECT_TRUE(vs_.Insert(z_));
  EXPECT_EQ(vs_.capacity(), 3u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
}

TEST_F(TestVariableSet, AddRangeSparse) {
  EXPECT_TRUE(vs_.Insert(y_));
  EXPECT_EQ(vs_.capacity(), 1u);
  vs_.Insert(std::initializer_list{z_, x_});
  EXPECT_EQ(vs_.capacity(), 3u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_TRUE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
}

TEST_F(TestVariableSet, AddBigRangeSparseLeft) {
  EXPECT_TRUE(vs_.Insert(x_));
  EXPECT_EQ(vs_.capacity(), 1u);
  vs_.Insert(std::initializer_list{w_});
  EXPECT_EQ(vs_.capacity(), 4u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
  EXPECT_FALSE(vs_.Contains(z_));
  EXPECT_TRUE(vs_.Contains(w_));
}

TEST_F(TestVariableSet, AddBigRangeSparseRight) {
  EXPECT_TRUE(vs_.Insert(w_));
  EXPECT_EQ(vs_.capacity(), 1u);
  vs_.Insert(std::initializer_list{x_});
  EXPECT_EQ(vs_.capacity(), 4u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
  EXPECT_FALSE(vs_.Contains(z_));
  EXPECT_TRUE(vs_.Contains(w_));
}

TEST_F(TestVariableSet, AddBigRangeSparseMiddleLeft) {
  EXPECT_TRUE(vs_.Insert(y_));
  EXPECT_EQ(vs_.capacity(), 1u);
  vs_.Insert(std::initializer_list{w_, x_});
  EXPECT_EQ(vs_.capacity(), 4u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_TRUE(vs_.Contains(y_));
  EXPECT_FALSE(vs_.Contains(z_));
  EXPECT_TRUE(vs_.Contains(w_));
}

TEST_F(TestVariableSet, AddBigRangeSparseMiddleRight) {
  EXPECT_TRUE(vs_.Insert(z_));
  EXPECT_EQ(vs_.capacity(), 1u);
  vs_.Insert(std::initializer_list{w_, x_});
  EXPECT_EQ(vs_.capacity(), 4u);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
  EXPECT_TRUE(vs_.Contains(w_));
}

TEST_F(TestVariableSet, AddMultipleVariables) {
  vs_.Insert(x_, y_, z_);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_TRUE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
  EXPECT_EQ(vs_.size(), 3u);
}

TEST_F(TestVariableSet, AddRangeVariables) {
  vs_.Insert(std::initializer_list{x_, y_, z_});
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_TRUE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
  EXPECT_EQ(vs_.size(), 3u);
}

TEST_F(TestVariableSet, RemoveSingleVariable) {
  vs_.Insert(x_);
  EXPECT_TRUE(vs_.Remove(x_));
  EXPECT_FALSE(vs_.Contains(x_));
  EXPECT_EQ(vs_.size(), 0u);
  EXPECT_FALSE(vs_.Remove(x_));
}

TEST_F(TestVariableSet, RemoveMultipleVariables) {
  vs_.Insert(x_, y_, z_);
  vs_.Remove(x_, y_);
  EXPECT_FALSE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
  EXPECT_EQ(vs_.size(), 1u);
}

TEST_F(TestVariableSet, RemoveRangeVariables) {
  vs_.Insert(x_, y_, z_);
  vs_.Remove(std::vector{x_, y_});
  EXPECT_FALSE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
  EXPECT_TRUE(vs_.Contains(z_));
  EXPECT_EQ(vs_.size(), 1u);
}

TEST_F(TestVariableSet, ContainsVariable) {
  vs_.Insert(x_);
  EXPECT_TRUE(vs_.Contains(x_));
  EXPECT_FALSE(vs_.Contains(y_));
}

TEST_F(TestVariableSet, GetVariable) {
  vs_.Insert(x_);
  EXPECT_TRUE(vs_.Get(x_.id()).equal_to(x_));
  EXPECT_THROW([[maybe_unused]] auto _ = vs_.Get(y_.id()), DelpiOutOfRangeException);
}

TEST_F(TestVariableSet, ClearVariables) {
  vs_.Insert(x_, y_, z_);
  vs_.Clear();
  EXPECT_TRUE(vs_.empty());
  EXPECT_EQ(vs_.size(), 0u);
}

TEST_F(TestVariableSet, MinMaxId) {
  vs_.Insert(x_, y_, z_);
  EXPECT_EQ(vs_.min_id(), x_.id());
  EXPECT_EQ(vs_.max_id(), z_.id() + 1);
}

TEST_F(TestVariableSet, CompareSet1) {
  std::set s{z_};
  VariableSet vs{z_};
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Insert(w_);
  s.insert(w_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Remove(w_, x_);
  s.erase(w_);
  s.erase(x_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Insert(std::vector{x_, w_});
  s.insert(x_);
  s.insert(w_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));
}

TEST_F(TestVariableSet, CompareSet2) {
  std::set s{x_, y_, z_};
  VariableSet vs{x_, y_, z_};
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Remove(x_);
  s.erase(x_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Insert(std::vector{w_, x_});
  s.insert(w_);
  s.insert(x_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));
}

TEST_F(TestVariableSet, CompareSet3) {
  std::set<Variable> s{x_, y_, z_};
  VariableSet vs{x_, y_, z_};
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Insert(w_);
  s.insert(w_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Insert(w_);
  s.insert(w_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Remove(x_);
  s.erase(x_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Remove(y_);
  s.erase(y_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Insert(x_);
  s.insert(x_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Insert(y_);
  s.insert(y_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Remove(z_);
  s.erase(z_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Remove(w_);
  s.erase(w_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));

  vs.Insert(z_);
  s.insert(z_);
  EXPECT_TRUE(std::ranges::equal(s, vs.variables(), CompareVariables));
}