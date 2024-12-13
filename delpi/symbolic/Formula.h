/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Formula class.
 */
#pragma once

#include "delpi/libs/gmp.h"
#include "delpi/symbolic/Expression.h"
#include "delpi/symbolic/FormulaKind.h"
#include "delpi/util/concepts.h"

namespace delpi {

class Formula {
 public:
  Formula(Expression expression, FormulaKind kind, mpq_class rhs);
  Formula(const Formula& o) = default;
  Formula(Formula&& o) noexcept = default;
  Formula& operator=(const Formula& o) = default;
  Formula& operator=(Formula&& o) noexcept = default;

  /**
   * Create a copy of this formula with the expression having
   * all occurrences of the variables in `s` replaced with the corresponding variable in `s`.
   *
   * Note that the substitutions occur simultaneously.
   * @code
   * Formula f = x + 2 * y == 0;
   * Substitution s = {{x, y}, {y, x}};
   * f.Substitute(s);  // returns y + 2 * x == 0
   * @endcode
   * @param s map of substitutions. Maps the old variable to the new one.
   * @return formula produced by the substitution
   */
  [[nodiscard]] Formula Substitute(const Expression::SubstitutionMap& s) const;
  /**
   * Evaluates the formula using a given environment (by default, an empty environment).
   * @tparam T map from variable to value (i.e. std::map<Variable, mpq_class>, std::unordered_map<Variable, mpq_class>)
   * @param env map between each variable and its value
   * @return true if the formula is satisfied within the given environment
   * @return false if the formula is not satisfied within the given environment
   * @throws std::exception if there exists variable in this expression whose assignment is not provided by `env`
   */
  template <MapFromTo<Variable, mpq_class> T>
  [[nodiscard]] bool Evaluate(const T& env) const;

  /** @getter{left-hand side expression, formula} */
  [[nodiscard]] const Expression& expression() const { return expression_; }
  /** @getter{kind @f$ \in \\{ =\, \ne\, \le\, \ge\, <\, > \\} @f$, formula} */
  [[nodiscard]] FormulaKind kind() const { return kind_; }
  /** @getter{right-hand side constant, formula} */
  [[nodiscard]] const mpq_class& rhs() const { return rhs_; }
  /** @getter{variables inside the expression, formula} */
  [[nodiscard]] std::vector<Variable> variables() const { return expression_.variables(); }

  /** @equal_to{formulas, The expressions in the formulas are equal when they have the same representation.} */
  [[nodiscard]] bool equal_to(const Formula& o) const noexcept;
  /** @less{formulas} */
  [[nodiscard]] bool less(const Formula& o) const noexcept;
  /** @hash{formula} */
  [[nodiscard]] std::size_t hash() const noexcept;

  Formula operator-() const;
  Formula operator!() const;
  std::strong_ordering operator<=>(const Formula&) const;
  bool operator==(const Formula& o) const { return equal_to(o); }

  Expression expression_;  ///< Left-hand side expression
  FormulaKind kind_;       ///< Kind of the formula @f$ \in \\{ =\, \ne\, \le\, \ge\, <\, > \\} @f$
  mpq_class rhs_;          ///< Right-hand side constant
};

Formula operator==(const Variable& lhs, const Variable& rhs);
Formula operator!=(const Variable& lhs, const Variable& rhs);
Formula operator<(const Variable& lhs, const Variable& rhs);
Formula operator<=(const Variable& lhs, const Variable& rhs);
Formula operator>(const Variable& lhs, const Variable& rhs);
Formula operator>=(const Variable& lhs, const Variable& rhs);

Formula operator==(const Expression& lhs, const Expression& rhs);
Formula operator!=(const Expression& lhs, const Expression& rhs);
Formula operator<(const Expression& lhs, const Expression& rhs);
Formula operator<=(const Expression& lhs, const Expression& rhs);
Formula operator>(const Expression& lhs, const Expression& rhs);
Formula operator>=(const Expression& lhs, const Expression& rhs);

Formula operator==(mpq_class lhs, const Expression& rhs);
Formula operator!=(mpq_class lhs, const Expression& rhs);
Formula operator<(mpq_class lhs, const Expression& rhs);
Formula operator<=(mpq_class lhs, const Expression& rhs);
Formula operator>(mpq_class lhs, const Expression& rhs);
Formula operator>=(mpq_class lhs, const Expression& rhs);

Formula operator==(const Expression& lhs, mpq_class rhs);
Formula operator!=(const Expression& lhs, mpq_class rhs);
Formula operator<(const Expression& lhs, mpq_class rhs);
Formula operator<=(const Expression& lhs, mpq_class rhs);
Formula operator>(const Expression& lhs, mpq_class rhs);
Formula operator>=(const Expression& lhs, mpq_class rhs);

std::ostream& operator<<(std::ostream& os, const Formula& formula);

}  // namespace delpi

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::Formula);

#endif
