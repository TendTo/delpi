/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/symbolic/Variable.h"

using delpi::Variable;

class TestVariable : public ::testing::Test {
 protected:
  const Variable d_{};
  const Variable x_{"x"};
  const Variable y_{"y"};
  const Variable z_{"z"};
};

TEST_F(TestVariable, Dummy) {
  EXPECT_TRUE(d_.is_dummy());
  EXPECT_FALSE(x_.is_dummy());
}

TEST_F(TestVariable, Name) {
  EXPECT_EQ(d_.name(), "dummy");
  EXPECT_EQ(x_.name(), "x");
  EXPECT_EQ(y_.name(), "y");
  EXPECT_EQ(z_.name(), "z");
}

TEST_F(TestVariable, Equality) {
  EXPECT_FALSE(d_.equal_to(x_));
  EXPECT_FALSE(x_.equal_to(d_));

  EXPECT_TRUE(x_.equal_to(x_));

  EXPECT_FALSE(x_.equal_to(y_));
  EXPECT_FALSE(x_.equal_to(z_));
  EXPECT_FALSE(y_.equal_to(z_));

  const Variable x_copy = x_;
  EXPECT_TRUE(x_.equal_to(x_copy));
  EXPECT_TRUE(x_copy.equal_to(x_));

  const Variable other_x{"x"};
  EXPECT_FALSE(x_.equal_to(other_x));
  EXPECT_FALSE(other_x.equal_to(x_));

  const Variable other_dummy{};
  EXPECT_TRUE(d_.equal_to(other_dummy));
  EXPECT_TRUE(other_dummy.equal_to(d_));
}

TEST_F(TestVariable, Hash) {
  EXPECT_NE(d_.hash(), x_.hash());
  EXPECT_NE(x_.hash(), y_.hash());
  EXPECT_NE(y_.hash(), z_.hash());

  EXPECT_EQ(d_.hash(), d_.hash());
  EXPECT_EQ(x_.hash(), x_.hash());
  EXPECT_EQ(y_.hash(), y_.hash());
  EXPECT_EQ(z_.hash(), z_.hash());
}

TEST_F(TestVariable, Less) {
  EXPECT_TRUE(x_.less(d_));
  EXPECT_TRUE(y_.less(d_));
  EXPECT_TRUE(z_.less(d_));

  EXPECT_TRUE(x_.less(y_));
  EXPECT_FALSE(y_.less(x_));

  EXPECT_TRUE(y_.less(z_));
  EXPECT_FALSE(z_.less(y_));

  EXPECT_FALSE(x_.less(x_));
  EXPECT_FALSE(y_.less(y_));
  EXPECT_FALSE(z_.less(z_));

  const Variable new_var{"new_var"};
  EXPECT_TRUE(x_.less(new_var));
  EXPECT_TRUE(y_.less(new_var));
  EXPECT_TRUE(z_.less(new_var));
  EXPECT_FALSE(d_.less(new_var));
}

TEST_F(TestVariable, Ostream) {
  EXPECT_EQ((std::stringstream{} << d_).str(), "dummy");
  EXPECT_EQ((std::stringstream{} << x_).str(), "x");
  EXPECT_EQ((std::stringstream{} << y_).str(), "y");
  EXPECT_EQ((std::stringstream{} << z_).str(), "z");
}

TEST_F(TestVariable, NoThrowMoveSintax) {
  static_assert(std::is_nothrow_move_constructible_v<Variable>, "Variable should be nothrow_move_constructible.");
  static_assert(std::is_nothrow_move_assignable_v<Variable>, "Variable should be nothrow_move_assignable.");
}

TEST_F(TestVariable, NoThrowComparators) {
  static_assert(std::is_nothrow_invocable_r_v<bool, decltype(&Variable::equal_to), const Variable&, const Variable&>,
                "Method equal_to should be is_nothrow_invocable.");
  static_assert(std::is_nothrow_invocable_r_v<bool, decltype(&Variable::less), const Variable&, const Variable&>,
                "Method less should be is_nothrow_invocable.");
  static_assert(std::is_nothrow_invocable_r_v<bool, decltype(&Variable::hash), const Variable&>,
                "Method hash should be is_nothrow_invocable");
}