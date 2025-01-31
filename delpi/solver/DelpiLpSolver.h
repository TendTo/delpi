/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * DelpiLpSolver class.
 */
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "delpi/libs/eigen.h"
#include "delpi/libs/gmp.h"
#include "delpi/solver/LpSolver.h"
#include "delpi/solver/internal/Basis.h"
#include "delpi/symbolic/Expression.h"
#include "delpi/symbolic/Variable.h"

namespace delpi {

/**
 * Linear programming solver using a custom implementation of the Simplex algorithm.
 * It is based on the [Bartels-Golub](https://doi.org/10.1007/BF02169151) method.
 */
class DelpiLpSolver final : public LpSolver {
 public:
  explicit DelpiLpSolver(Config config = {}, const std::string& class_name = "DelpiLpSolver");

  [[nodiscard]] int num_columns() const override;
  [[nodiscard]] int num_rows() const override;

  [[nodiscard]] const Matrix<mpq_class>& A() const { return A_; }
  [[nodiscard]] const Vector<mpq_class>& c() const { return c_; }
  [[nodiscard]] const Vector<mpq_class>& b() const { return b_; }
  [[nodiscard]] const Vector<mpq_class>& x() const { return x_; }

  [[nodiscard]] Column column(ColumnIndex column_idx) const override;
  [[nodiscard]] Row row(RowIndex row_idx) const override;
  void ReserveColumns(int num_columns) override;
  void ReserveRows(int num_rows) override;
  ColumnIndex AddColumn(const Variable& var, const mpq_class& obj, const mpq_class& lb, const mpq_class& ub) override;
  RowIndex AddRow(const std::vector<Expression::Addend>& addends, const mpq_class& lb, const mpq_class& ub) override;
  RowIndex AddRow(const Expression::Addends& lhs, FormulaKind sense, const mpq_class& rhs) override;
  void SetBound(Variable var, const mpq_class& lb, const mpq_class& ub) override;
  void SetCoefficient(RowIndex row, ColumnIndex column, const mpq_class& value) override;
  void SetObjective(int column, const mpq_class& value) override;

#ifndef NDEBUG
  void Dump() override;
#endif

 private:
  LpResult SolveCore(mpq_class& precision, bool store_solution) override;
  /**
   * Solve the LP problem using the Simplex algorithm.
   * The input is assumed to be in standard form, i.e.
   * @f[
   * \begin{array}{rl}
   * \min & c^T x \\
   * \text{s.t.} & Ax = b \\
   * & x \ge 0
   * \end{array}
   * @f]
   * @pre The LP problem must be in standard form.
   * @pre `A` must have the same number of rows as `b` and the same number of columns as `c`.
   * @pre `A` must have the same number of rows as `basis` has columns (and rows, since `basis` is a square matrix).
   * @pre `basis` must be a feasible basis for the problem.
   * @param A coefficient matrix
   * @param b rhs vector
   * @param c objective function
   * @param basis starting feasible basis
   * @return LpResult::OPTIMAL if the problem is feasible, bounded and an optimal solution has been found
   * @return LpResult::UNBOUNDED if the problem is unbounded
   */
  LpResult InternalSolve(const Matrix<mpq_class>& A, const Vector<mpq_class>& b, const Vector<mpq_class>& c,
                         internal::Basis<mpq_class>& basis);
  // TODO(tend): add mpf_class support
  // LpResult InternalSolve(const Matrix<T>& A, const Vector<T>& b, const Vector<T>& c, internal::Basis<T>& basis);
  LpResult FeasibilityCheck(Matrix<mpq_class>& slack_A, Vector<mpq_class>& slack_b,
                            internal::Basis<mpq_class>& slack_basis);
  LpResult OptimalityCheck(const internal::Basis<mpq_class>& basis);
  LpResult UnboundednessCheck(const internal::Basis<mpq_class>& basis);
  /**
   * Use `aux_basis` to update `basis` with a set of columns we know feasible.
   * If `aux_basis` still contains any auxiliary columns, they will be removed from the `basis`
   * and the corresponding row in `A` and `b` will be removed as well, thus reducing the size of the problem.
   * @pre the auxiliary columns must be the rightmost columns the `aux_basis`' coefficient matrix.
   * @param aux_basis basis obtained from the auxiliary problem that produces an objective value of 0
   * @param[in,out] slack_A coefficient matrix of the standard form problem
   * @param[out] slack_b rhs vector of the standard form problem
   * @param[out] slack_basis basis associated with the standard form problem to be updated
   */
  void RemoveAuxiliaryColumns(const internal::Basis<mpq_class>& aux_basis, Matrix<mpq_class>& slack_A,
                              Vector<mpq_class>& slack_b, internal::Basis<mpq_class>& slack_basis) const;

  /**
   * Convert a generic LP problem into the standard form by adding slack variables where needed.
   * A generic LP problem is defined as:
   * @f[
   * \begin{array}{rl}
   * \min & c^T x \\
   * \text{s.t.} & l_r \le Ax \le u_r \\
   * & l_c \le x \le u_c
   * \end{array}
   * @f]
   * whereas the standard form is:
   * @f[
   * \begin{array}{rl}
   * \min & c^T x \\
   * \text{s.t.} & Ax = b \\
   * & x \ge 0
   * \end{array}
   * @f]
   * It does so by introducing slack variables for each row of the original problem where the sense is not equality.
   * The slack variables are non-negative and have a positive coefficient if the sense is less than or equal,
   * and a negative coefficient if the sense is greater than or equal.
   * Since equality constraints are already in the standard form, no slack variable is needed.
   * @pre @ref ComputeSlackAndAuxVariables must have been called before this method.
   * @param[out] slack_A coefficient matrix to be set in the standard form with respect to @ref A_
   * @param[out] slack_c objective function coefficients to be set in the standard form with respect to @ref c_
   */
  void SlackForm(Matrix<mpq_class>& slack_A, Vector<mpq_class>& slack_c) const;

  /**
   * Convert a standard form LP problem into a feasible and bounded auxiliary problem.
   * It does so by introducing auxiliary variables for each row of the standard form problem where either no slack
   * variable is present (i.e. the sense was originally equality)
   * or the slack variable's coefficient and the right-hand side have different signs.
   * @pre @ref ComputeSlackAndAuxVariables must have been called before this method.
   * @param slack_A coefficient matrix of the problem in standard form
   * @param[out] aux_A coefficient matrix to be set in the auxiliary form with respect to `std_A`
   * @param[out] aux_c objective function coefficients to be set in the auxiliary form with respect to `std_A`
   * @return feasible and bounded basis for the auxiliary problem
   */
  internal::Basis<mpq_class> AuxForm(const Matrix<mpq_class>& slack_A, Matrix<mpq_class>& aux_A,
                                     Vector<mpq_class>& aux_c) const;

  /**
   * Compute the slack and auxiliary variables for the original LP problem stored in @ref A_, @ref b_ and @ref c_
   * in order to convert it to the standard form and check its feasibility.
   * A generic LP problem is defined as:
   * @f[
   * \begin{array}{rl}
   * \min & c^T x \\
   * \text{s.t.} & l_r \le Ax \le u_r \\
   * & l_c \le x \le u_c
   * \end{array}
   * @f]
   * whereas the standard form is:
   * @f[
   * \begin{array}{rl}
   * \min & c^T x \\
   * \text{s.t.} & Ax = b \\
   * & x \ge 0
   * \end{array}
   * @f]
   * It does so by introducing slack variables for each row of the original problem where the sense is not equality.
   * The slack variables are non-negative and have a positive coefficient if the sense is less than or equal,
   * and a negative coefficient if the sense is greater than or equal.
   * Since equality constraints are already in the standard form, no slack variable is needed.
   * To know which slack variable corresponds to which row, the method stores the indices of the (slack columns + 1) in
   * the @ref slack_columns_ vector, with the index being positive if the coefficient of the slack variable is 1,
   * negative otherwise.
   * The method also stores the indices of the will-be auxiliary columns in the @ref aux_columns_ vector in a similar
   * fashion, with the sign of the index being the same as the sign of its coefficient
   * Auxiliary variables are added to each row where either no slack variable has been added or the slack variable's
   * coefficient and the right-hand side have different signs.
   * @note the @ref aux_columns_ vector is cleared before storing the new indices.
   * @note the @ref slack_columns_ vector is cleared before storing the new indices.
   * @warning in order to save space, the values stored in @ref slack_columns_ and @ref aux_columns_ are the indices + 1
   * and the sign of the index is the same as the sign of the coefficient.
   */
  void ComputeSlackAndAuxVariables();
#if 0
  /**
   * Use the result from the lp solver to update the infeasible ray with the conflict that has been detected.
   *
   * This will allow the SAT solver to find a new assignment without the conflict.
   * On the other hand, both @ref solution_ and @ref dual_solution_ will be cleared.
   *
   * More formally, we can use the infeasible ray @f$ y @f$ to create the linear inequality @f$ (y^T A) x \le y^T b @f$,
   * which is infeasible over the local bounds.
   * In other words, even setting each element of @f$ x @f$ to the bound that minimise @f$ (y^A) x @f$,
   * its value is still greater than @f$ y^T b @f$.
   */
  void UpdateInfeasible();
#endif

  Matrix<mpq_class> A_;                  ///< Coefficient matrix
  Vector<mpq_class> c_;                  ///< Objective function coefficients
  Vector<mpq_class> b_;                  ///< Right-hand side
  Vector<mpq_class> x_;                  ///< Solution
  Vector<mpq_class> y_;                  ///< Dual solution
  std::vector<FormulaKind> row_senses_;  ///< Row senses (i.e. <=, =, >=)
  std::vector<Index> slack_columns_;     ///< Indices of the slack columns. Element at index `i` is the slack column for
                                         ///< row `slack_columns_[i]`
  std::vector<Index> aux_columns_;  ///< Indices of the auxiliary columns. Element at index `i` is the auxiliary column
                                    ///< for row `aux_columns_[i]`
};

std::ostream& operator<<(std::ostream& os, const DelpiLpSolver& solver);

}  // namespace delpi
