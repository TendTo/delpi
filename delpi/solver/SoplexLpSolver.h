/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * SoplexLpSolver class.
 */
#pragma once

#ifndef DELPI_ENABLED_SOPLEX
#error SoPlex is not enabled. Please enable it by adding "--//tools:enable_soplex" to the bazel command.
#endif

#include "delpi/libs/gmp.h"
#include "delpi/libs/soplex.h"
#include "delpi/solver/LpSolver.h"
#include "delpi/symbolic/Expression.h"
#include "delpi/symbolic/Variable.h"
#include "delpi/util/concepts.h"

namespace delpi {

/**
 * Linear programming solver using [SoPlex](https://soplex.zib.de/).
 */
class SoplexLpSolver final : public LpSolver {
 public:
  explicit SoplexLpSolver(const Config& config, const std::string& class_name = "SoplexLpSolver");

  [[nodiscard]] int num_columns() const override;
  [[nodiscard]] int num_rows() const override;

  [[nodiscard]] Column column(ColumnIndex column_idx) const override;
  [[nodiscard]] Row row(RowIndex row_idx) const override;
  void ReserveColumns(int num_columns) override;
  void ReserveRows(int num_rows) override;
  ColumnIndex AddColumn(const Variable& var, const mpq_class& obj, const mpq_class& lb, const mpq_class& ub) override;
  RowIndex AddRow(const Row& row) override;
  RowIndex AddRow(const Expression::Addends& lhs, FormulaKind sense, const mpq_class& rhs) override;
  void SetCoefficient(RowIndex row, ColumnIndex column, const mpq_class& value) override;
  void SetObjective(int column, const mpq_class& value) override;

#ifndef NDEBUG
  void Dump() override;
#endif

 private:
  LpResult SolveCore(mpq_class& precision, bool store_solution) override;
  /**
   * Parse a sequence of `literal_monomials` and set the coefficient for each decisional variable appearing in it.
   * @tparam TypedIterable generic iterable containing pairs (Variable, coeff) (i.e. std::vector, std::set, std::span)
   * @param literal_monomials symbolic formula representing the row
   */
  template <TypedIterable<std::pair<const Variable, mpq_class>> T>
  soplex::DSVectorRational ParseRowCoeff(const T& literal_monomials);
  /**
   * Set the coefficients to apply to `var` on a specific row.
   *
   * The coefficient is set in `coeff`.
   * @param coeffs[out] vector of coefficients to apply to the decisional variables
   * @param var variable to set the coefficients for
   * @param value value to set the coefficients to
   */
  void SetVarCoeff(soplex::DSVectorRational& coeffs, const Variable& var, const mpq_class& value) const;

  /**
   * Use the result from the lp solver to update the solution vector and objective value.
   *
   * The lp solver was able to find a feasible solution to the problem.
   * The useful information will be stored in @ref solution_ and @ref dual_solution_.
   * On the other hand, both @ref infeasible_rows_ and @ref infeasible_bounds_ will be cleared.
   */
  void UpdateFeasible();
#if 0
  /**
   * Use the result from the lp solver to update the infeasible ray with the conflict that has been detected.
   *
   * This will allow the SAT solver to find a new assignment without the conflict.
   * The useful information will be stored in @ref infeasible_rows_ and @ref infeasible_bounds_.
   * On the other hand, both @ref solution_ and @ref dual_solution_ will be cleared.
   *
   * More formally, we can use the infeasible ray @f$ y @f$ to create the linear inequality @f$ (y^T A) x \le y^T b @f$,
   * which is infeasible over the local bounds.
   * In other words, even setting each element of @f$ x @f$ to the bound that minimise @f$ (y^A) x @f$,
   * its value is still greater than @f$ y^T b @f$.
   */
  void UpdateInfeasible();
#endif

 private:
  bool consolidated_;  ///< Whether the LP problem has been consolidated

  soplex::SoPlex spx_;  ///< SoPlex LP solver

  soplex::LPColSetRational spx_cols_;  ///< Columns of the LP problem
  soplex::LPRowSetRational spx_rows_;  ///< Rows of the LP problem

  soplex::Rational rninfinity_;  ///< Rational negative infinity
  soplex::Rational rinfinity_;   ///< Rational positive infinity
};

}  // namespace delpi
