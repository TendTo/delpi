/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * QsoptexLpSolver class.
 */
#pragma once

#ifndef DELPI_ENABLED_QSOPTEX
#error QSopt_ex is not enabled. Please enable it by adding "--//tools:enable_qsoptex" to the bazel command.
#endif

#include "delpi/libs/gmp.h"
#include "delpi/libs/qsopt_ex.h"
#include "delpi/solver/LpSolver.h"
#include "delpi/symbolic/Expression.h"
#include "delpi/symbolic/Variable.h"
#include "delpi/util/concepts.h"

namespace delpi {

/**
 * Linear programming solver using [QSopt_ex](https://www.math.uwaterloo.ca/~bico/qsopt/ex/).
 */
class QsoptexLpSolver final : public LpSolver {
 public:
  explicit QsoptexLpSolver(Config config = {}, const std::string& class_name = "QsoptexLpSolver");
  ~QsoptexLpSolver() override;

  [[nodiscard]] int num_columns() const override;
  [[nodiscard]] int num_rows() const override;

  [[nodiscard]] Column column(int column_idx) const override;
  [[nodiscard]] Row row(int row_idx) const override;
  ColumnIndex AddColumn(const Variable& var, const mpq_class& obj, const mpq_class& lb, const mpq_class& ub) override;
  RowIndex AddRow(const Row& row) override;
  RowIndex AddRow(const Expression::Addends& lhs, FormulaKind sense, const mpq_class& rhs) override;
  void SetCoefficient(RowIndex row, ColumnIndex column, const mpq_class& value) override;
  void SetObjective(int column, const mpq_class& value) override;

#ifndef NDEBUG
  void Dump() final;
#endif

 private:
  LpResult SolveCore(mpq_class& precision, bool store_solution) override;

  /**
   * Parse a sequence of `literal_monomials` and set the coefficient for each decisional variable appearing in it.
   * @tparam TypedIterable generic iterable containing pairs (Variable, coeff) (i.e. std::vector, std::set, std::span)
   * @param row row to set the coefficients for
   * @param literal_monomials symbolic formula representing the row
   */
  template <TypedIterable<std::pair<const Variable, mpq_class>> T>
  void SetRowCoeff(int row, const T& literal_monomials);
  /**
   * Set the coefficients to apply to `var` on a specific `row`.
   * @param row row to set the coefficients for
   * @param var variable to set the coefficients for
   * @param value value to set the coefficients to
   */
  void SetVarCoeff(int row, const Variable& var, const mpq_class& value) const;

  /**
   * Use the result from the lp solver to update the solution vector and objective value.
   *
   * The lp solver was able to find a feasible solution to the problem.
   * The useful information will be stored in @ref objective_value_ and @ref solution_.
   * On the other hand, both @ref infeasible_rows_ and @ref infeasible_bounds_ will be cleared.
   */
  void UpdateFeasible();
#if 0
  /**
   * Use the result from the lp solver to update the infeasible ray with the conflict that has been detected.
   *
   * This will allow the SAT solver to find a new assignment without the conflict.
   * The useful information will be stored in @ref infeasible_rows_ and @ref infeasible_bounds_.
   * On the other hand, both @ref objective_value_ and @ref solution_ will be cleared.
   *
   * More formally, we can use the infeasible ray @f$ y @f$ to create the linear inequality @f$ (y^T A) x \le y^T b @f$,
   * which is infeasible over the local bounds.
   * In other words, even setting each element of @f$ x @f$ to the bound that minimise @f$ (y^A) x @f$,
   * its value is still greater than @f$ y^T b @f$.
   */
  void UpdateInfeasible();
#endif

 private:
  mpq_QSprob qsx_;  ///< QSopt_ex LP solver

  qsopt_ex::MpqArray ray_;  ///< Ray of the last infeasible solution
  qsopt_ex::MpqArray x_;    ///< Solution vector
};

}  // namespace delpi
