/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence Apache-2.0 license
 * ExpressionCell class.
 */
#pragma once

#include "delpi/symbolic/Expression.h"
#include "delpi/util/SelfReferenceCountingObject.hpp"
#include "delpi/util/intrusive_ptr.hpp"

namespace delpi {

/**
 * Symbolic expression representing an addition between linear monomials.
 *
 * It can be used to encode expression of the kind
 * @f[
 *     \sum c_i \cdot x_i
 * @f]
 * where @f$ c_i @f$ is a constant and @f$ x_i @f$ is a Variable.
 * Internally this class maintains a member variable @c addends_
 * to represent a mapping between each variable @f$ x_i @f$ to its rational @f$ c_i @f$.
 */
class ExpressionCell : public SelfReferenceCountingObject {
 public:
  using Addends = Expression::Addends;
  using Environment = Expression::Environment;
  using SubstitutionMap = Expression::SubstitutionMap;

  /** @getter{variables, expression} */
  [[nodiscard]] std::vector<Variable> variables() const;
  /** @getter{variables, expression} */
  [[nodiscard]] const Addends& addends() const noexcept { return addends_; }

  /** @equal_to{expression cells, Two expressions are structurally equal when they have the same representation.} */
  [[nodiscard]] bool equal_to(const ExpressionCell& o) const noexcept;
  /** @less{expression cells} */
  [[nodiscard]] bool less(const ExpressionCell& o) const noexcept;
  /** @hash{expression cell} */
  [[nodiscard]] std::size_t hash() const noexcept;

  /**
   * Add a linear monomial @f$ c \cdot x @f$,
   * where @f$ c @f$ is a constant and @f$ x @f$ is a Variable, to the current expression.
   * @param var variable of the linear monomial
   * @param coeff coefficient of the linear monomial
   * @return reference to this object
   */
  ExpressionCell& Add(const Variable& var, const mpq_class& coeff);
  /**
   * Multiply all terms of the summation by a `coeff`.
   * @param coeff coefficient all the terms will be mutiplied by
   * @return reference to this object
   */
  ExpressionCell& Multiply(const mpq_class& coeff);
  /**
   * Divide all terms of the summation by a `coeff`.
   * @param coeff coefficient all the terms will be divided by
   * @return reference to this object
   * @throws delpi::DelpiException division by 0 detected
   */
  ExpressionCell& Divide(const mpq_class& coeff);

  /**
   * Evaluates using a given environment (by default, an empty environment).
   * @throws std::exception if there exists a variable in this expression whose assignment is not provided by `env`
   */
  [[nodiscard]] mpq_class Evaluate(const Environment& env = {}) const;

  /**
   * Create a copy of this expression,
   * replacing all occurrences of the variables in `s` with corresponding expressions in `s`.
   *
   * Note that the substitutions occur simultaneously.
   * @code
   * Expression e = x + 5 * y;
   * Substitution s = {{x, y}, {y, x}};
   * e.Substitute(s);  // returns y + 5 * x
   * @endcode
   */
  [[nodiscard]] Expression Substitute(const SubstitutionMap& s) const;

  std::ostream& Print(std::ostream& os) const;
  static std::ostream& PrintAddend(std::ostream& os, bool print_plus, const Variable& var, const mpq_class& coeff);

  static intrusive_ptr<ExpressionCell> New();
  static intrusive_ptr<ExpressionCell> New(Variable var);
  static intrusive_ptr<ExpressionCell> New(LinearMonomial linear_monomial);
  static intrusive_ptr<ExpressionCell> New(Addends addends);
  static intrusive_ptr<ExpressionCell> Copy(const ExpressionCell& o);

 protected:
  ExpressionCell() = default;
  explicit ExpressionCell(Variable var);
  explicit ExpressionCell(LinearMonomial linear_monomial);
  explicit ExpressionCell(Addends addends);

  mutable std::size_t hash_;     ///< Cached hash of the object
  Expression::Addends addends_;  ///< Map between each variable and it coefficient as terms of the summation
};

}  // namespace delpi
