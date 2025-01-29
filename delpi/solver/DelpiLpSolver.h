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
  LpResult FeasibilityCheck(const Matrix<mpq_class>& A, internal::Basis<mpq_class>& basis);
  LpResult OptimalityCheck(const internal::Basis<mpq_class>& basis);
  LpResult UnboundednessCheck(const internal::Basis<mpq_class>& basis);
  /**
   * Check if `basis` contains any auxiliary columns and remove them, replacing them with valid columns from `A`.
   * @pre The auxiliary columns must be the leftmost columns of `A`.
   * @pre The number of auxiliary columns must be less than or equal to the number of actual columns.
   * @param A coefficient matrix containing the auxiliary columns
   * @param basis basis possibly containing the auxiliary columns
   * @param num_columns_to_keep number of rightmost columns of `A` that will be kept. If negative, it will be set to
   * the number of rows of `A`.
   */
  void RemoveAuxiliaryColumns(const Matrix<mpq_class>& A, internal::Basis<mpq_class>& basis,
                              int num_columns_to_keep = -1) const;

  std::pair<Matrix<mpq_class>, Vector<mpq_class>> ToSlackForm() const;
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
};

std::ostream& operator<<(std::ostream& os, const DelpiLpSolver& solver);

}  // namespace delpi
