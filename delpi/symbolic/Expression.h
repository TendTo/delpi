/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence Apache-2.0 license
 * Expression class.
 */
#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "LinearMonomial.h"
#include "delpi/libs/gmp.h"
#include "delpi/symbolic/Variable.h"
#include "delpi/util/hash.hpp"

namespace delpi {

/**
 * Represents a symbolic form of an expression.
 *
 * Its syntax tree is as follows:
 *
 * @verbatim
 * E := Var | Constant | E + ... + E | Constant * E | inf | NaN
 * @endverbatim
 * In the implementation, Expression directly stores an intrusive_ptr to a const ExpressionCell class
 * that is a super-class of different kinds of symbolic expressions (e.g. ExpressionAdd, ExpressionConst, ...),
 * which makes it efficient to copy, move, and assign to another Expression.
 * @note -E is represented as -1 * E internally.
 * @note A subtraction E1 - E2 is represented as E1 + (-1 * E2) internally.
 * The following simple simplifications are implemented:
 * @verbatim
 * E + 0             ->  E
 * 0 + E             ->  E
 * E - 0             ->  E
 * E - E             ->  0
 * E * 1             ->  E
 * 1 * E             ->  E
 * E * 0             ->  0
 * 0 * E             ->  0
 * @endverbatim
 * Constant folding is implemented:
 * @verbatim
 * E(c1) + E(c2)  ->  E(c1 + c2)    // c1, c2 are constants
 * E(c1) - E(c2)  ->  E(c1 - c2)
 * E(c1) * E(c2)  ->  E(c1 * c2)
 * E(c1) / E(c2)  ->  E(c1 / c2)
 * @endverbatim
 * Relational operators over expressions (==, !=, <, >, <=, >=) return a Formula instead of bool.
 * Those operations are declared in formula.h file.
 * To check structural equality between two expressions use @ref Expression::equal_to.
 */
class Expression {
 public:
  using Addends = std::map<Variable, mpq_class>;
  using Environment = std::map<Variable, mpq_class>;

  /** @constructor{expression, Default to zero} */
  Expression() = default;
  Expression(const LinearMonomial& linear_monomial);  // NOLINT (runtime/explicit): This conversion is desirable.
  explicit Expression(Addends addends);

  /** @getter{variables, expression} */
  [[nodiscard]] std::vector<Variable> variables() const;

  /**
   * Checks structural equality.
   *
   * Two expressions e1 and e2 are structurally equal when they have the same
   * internal AST(abstract-syntax tree) representation. Please note that we can
   * have two computationally (or extensionally) equivalent expressions which
   * are not structurally equal. For example, consider:
   * @f[
   *    e1 = 2 * (x + y) \\
   *    e2 = 2x + 2y
   * @f]
   * Obviously, we know that e1 and e2 are evaluated to the same value for all
   * assignments to x and y. However, e1 and e2 are not structurally equal by
   * the definition. Note that e1 is a multiplication expression
   * (is_multiplication(e1) is true) while e2 is an addition expression
   * (is_addition(e2) is true).
   *
   * Note that for polynomial cases, you can use Expand method and check if two
   * polynomial expressions p1 and p2 are computationally equal. To do so, you
   * check the following:
   * @code
   *     p1.Expand().EqualTo(p2.Expand())
   * @endcode
   */
  [[nodiscard]] bool equal_to(const Expression& e) const;
  /** @less{expressions} */
  [[nodiscard]] bool less(const Expression& e) const;
  /** @hash{expression} */
  [[nodiscard]] std::size_t hash() const noexcept { return hash_value<Addends>{}(addends_); }

  /**
   * Evaluates using a given environment (by default, an empty environment).
   * @throws std::exception if there exists a non-random variable in this expression
   * whose assignment is not provided by @p env
   * @throws std::exception if an unassigned random variable is detected while @p random_generator is `nullptr`
   * @throws std::exception if NaN is detected during evaluation.
   */
  [[nodiscard]] mpq_class Evaluate(const Environment& env = {}) const;

  /**
   * Returns a copy of this expression,
   * replacing all occurrences of the variables in @p s with corresponding expressions in @p s.
   *
   * Note that the substitutions occur simultaneously.
   * @code
   * Expression e = x / y;
   * Substitution s = {{x, y}, {y, x}};
   * e.Substitute(s);  // returns y / x
   * @endcode
   * @throws std::exception if NaN is detected during substitution.
   */
  [[nodiscard]] Expression Substitute(const std::unordered_map<Variable, Variable>& s) const;
  /** @getter{string representation, expression}. */
  [[nodiscard]] std::string ToString() const;

  Expression& Add(const Variable& var, const mpq_class& val);
  Expression& Subtract(const Variable& var, const mpq_class& val);

  Expression& operator*=(const mpq_class& o);
  Expression& operator/=(const mpq_class& o);
  Expression operator*(const mpq_class& o) const;
  Expression operator/(const mpq_class& o) const;

  Expression& operator+=(const Variable& o);
  Expression& operator-=(const Variable& o);
  Expression operator+(const Variable& o) const;
  Expression operator-(const Variable& o) const;
  Expression& operator+=(const LinearMonomial& o);
  Expression& operator-=(const LinearMonomial& o);
  Expression operator+(const LinearMonomial& o) const;
  Expression operator-(const LinearMonomial& o) const;
  Expression& operator+=(const Expression& o);
  Expression& operator-=(const Expression& o);
  Expression operator+(const Expression& o) const;
  Expression operator-(const Expression& o) const;

  Addends addends_;
};

Expression operator*(const mpq_class& o, const Expression& e);

Expression operator+(const Variable& o, const Expression& e);
Expression operator-(const Variable& o, const Expression& e);
Expression operator+(const LinearMonomial& o, const Expression& e);
Expression operator-(const LinearMonomial& o, const Expression& e);

std::ostream& operator<<(std::ostream& os, const Expression& e);

}  // namespace delpi

/* Provides std::hash<smats::Expression>. */
template <>
struct std::hash<delpi::Expression> {
  size_t operator()(const delpi::Expression& val) const noexcept { return val.hash(); }
};

/* Provides std::less<smats::Expression>. */
template <>
struct std::less<delpi::Expression> {
  bool operator()(const delpi::Expression& lhs, const delpi::Expression& rhs) const { return lhs.less(rhs); }
};

/* Provides std::equal_to<smats::Expression>. */
template <class T>
struct std::equal_to<delpi::Expression> {
  bool operator()(const delpi::Expression& lhs, const delpi::Expression& rhs) const { return lhs.equal_to(rhs); }
};

