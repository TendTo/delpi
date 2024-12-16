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
#include <utility>
#include <vector>

#include "delpi/libs/gmp.h"
#include "delpi/symbolic/Variable.h"
#include "delpi/util/concepts.h"
#include "delpi/util/intrusive_ptr.hpp"

namespace delpi {

// Forward declaration. Found in "delpi/symbolic/ExpressionCell.h"
class ExpressionCell;

/**
 * Represents a symbolic form of an expression.
 * Its syntax tree is as follows:
 * ```
 * E := Var | E + ... + E | Constant * E
 * ```
 * To check structural equality between two expressions use @ref Expression::equal_to.
 */
class Expression {
 public:
  using Addend = std::pair<Variable, mpq_class>;
  using Addends = std::map<Variable, mpq_class>;
  using SubstitutionMap = std::unordered_map<Variable, Variable>;

  /** @constructor{expression, Default to zero} */
  Expression();
  Expression(Variable var);   // NOLINT (runtime/explicit): This conversion is desirable.
  Expression(Addend addend);  // NOLINT (runtime/explicit): This conversion is desirable.
  explicit Expression(Addends addends);
  explicit Expression(std::vector<Addend> addends);
  Expression(const Expression& e);
  Expression(Expression&& e) noexcept;
  Expression& operator=(const Expression& e);
  Expression& operator=(Expression&& e) noexcept;
  ~Expression();

  /** @getter{variables, expression} */
  [[nodiscard]] std::vector<Variable> variables() const;
  /** @getter{variables, expression} */
  [[nodiscard]] const Addends& addends() const;
  /** @getter{reference counter, underlying expression cell} */
  [[nodiscard]] std::size_t use_count() const;

  /** @equal_to{expressions, Two expressions are structurally equal when they have the same representation.} */
  [[nodiscard]] bool equal_to(const Expression& o) const noexcept;
  /** @less{expressions} */
  [[nodiscard]] bool less(const Expression& o) const noexcept;
  /** @hash{expression} */
  [[nodiscard]] std::size_t hash() const noexcept;
  /**
   * Evaluates using a given environment (by default, an empty environment).
   * @tparam T map from variable to value (i.e. std::map<Variable, mpq_class>, std::unordered_map<Variable, mpq_class>)
   * @param env map between each variable and its value
   * @return value of the expression in the given environment
   * @throws std::exception if there exists variable in this expression whose assignment is not provided by `env`
   */
  template <MapFromTo<Variable, mpq_class> T>
  [[nodiscard]] mpq_class Evaluate(const T& env = {}) const;

  /**
   * Create a copy of this expression,
   * replacing all occurrences of the variables in `s` with corresponding expressions in `s`.
   *
   * Note that the substitutions occur simultaneously.
   * @code
   * Expression e = x + 2 * y;
   * Substitution s = {{x, y}, {y, x}};
   * e.Substitute(s);  // returns y + 2 * x
   * @endcode
   * @param s map of substitutions. Maps the old variable to the new one.
   * @return expression produced by the substitution
   */
  [[nodiscard]] Expression Substitute(const SubstitutionMap& s) const;
  /** @getter{string representation, expression} */
  [[nodiscard]] std::string ToString() const;

  /**
   * Add a linear monomial @f$ c \cdot x @f$,
   * where @f$ c @f$ is a constant and @f$ x @f$ is a Variable, to the current expression.
   * @param var variable of the linear monomial
   * @param coeff coefficient of the linear monomial
   * @return reference to this object
   */
  Expression& Add(const Variable& var, const mpq_class& coeff);
  /**
   * Subtract a linear monomial @f$ c \cdot x @f$,
   * where @f$ c @f$ is a constant and @f$ x @f$ is a Variable, to the current expression.
   * @param var variable of the linear monomial
   * @param coeff coefficient of the linear monomial
   * @return reference to this object
   */
  Expression& Subtract(const Variable& var, const mpq_class& coeff);

  Expression operator-() const;
  Expression operator+() const;

  Expression& operator*=(const mpq_class& o);
  Expression& operator/=(const mpq_class& o);
  Expression operator*(const mpq_class& o) const;
  Expression operator/(const mpq_class& o) const;

  Expression& operator+=(const Variable& o);
  Expression& operator-=(const Variable& o);
  Expression operator+(const Variable& o) const;
  Expression operator-(const Variable& o) const;
  Expression& operator+=(const Addend& o);
  Expression& operator-=(const Addend& o);
  Expression operator+(const Addend& o) const;
  Expression operator-(const Addend& o) const;
  Expression& operator+=(const Expression& o);
  Expression& operator-=(const Expression& o);
  Expression operator+(const Expression& o) const;
  Expression operator-(const Expression& o) const;

  /**
   * Print a string representation of this class to the provided `os`.
   *
   * It is an alternative to the more standard operator<<.
   * @param os output stream
   * @return reference to the output stream
   */
  std::ostream& Print(std::ostream& os) const;

 private:
  intrusive_ptr<ExpressionCell> ptr_;  ///< Internal pointer to the ExpressionCell
};

Expression operator-(const Variable& var);

Expression operator*(const mpq_class& lhs, const Expression& rhs);
Expression operator*(const mpq_class& lhs, const Variable& rhs);
Expression operator*(const Variable& lhs, const mpq_class& rhs);
Expression operator/(const Variable& lhs, const mpq_class& rhs);

Expression operator+(const Variable& lhs, const Expression& rhs);
Expression operator-(const Variable& lhs, const Expression& rhs);
Expression operator+(const Expression::Addend& lhs, const Expression& rhs);
Expression operator-(const Expression::Addend& lhs, const Expression& rhs);

std::ostream& operator<<(std::ostream& os, const Expression& e);

}  // namespace delpi

/* Provides std::hash<smats::Expression>. */
template <>
struct std::hash<delpi::Expression> {
  size_t operator()(const delpi::Expression& e) const noexcept { return e.hash(); }
};

/* Provides std::less<smats::Expression>. */
template <>
struct std::less<delpi::Expression> {
  bool operator()(const delpi::Expression& lhs, const delpi::Expression& rhs) const noexcept { return lhs.less(rhs); }
};

/* Provides std::equal_to<smats::Expression>. */
template <>
struct std::equal_to<delpi::Expression> {
  bool operator()(const delpi::Expression& lhs, const delpi::Expression& rhs) const noexcept {
    return lhs.equal_to(rhs);
  }
};

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::Expression);

#endif
