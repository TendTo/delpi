/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * LpSolver class.
 */
#pragma once

#include <iosfwd>
#include <memory>
#include <unordered_map>

#include "delpi/libs/gmp.h"
#include "delpi/solver/LpResult.h"
#include "delpi/solver/LpRowSense.h"
#include "delpi/symbolic/Expression.h"
#include "delpi/symbolic/Formula.h"
#include "delpi/symbolic/Variable.h"
#include "delpi/util/Config.h"
#include "delpi/util/Stats.h"

namespace delpi {

/**
 * Facade class that hides the underlying LP solver used by delpi.
 *
 * It provides a common interface to interact with any number of LP solvers, implemented as subclasses.
 * An LP problem is defined as
 * @f[
 * \begin{array}{}
 *      & \max              & c^T x     \newline
 *      & \text{subject to} & A x \le b \newline
 *      &                   & l \le x \le u
 * \end{array}
 * @f]
 * where @f$ x @f$ is the vector of real variables, @f$ c @f$ is the vector of objective coefficients,
 * @f$ A @f$ is the matrix of coefficients of the constraints, @f$ b @f$ is the right-hand side of the constraints,
 * @f$ l @f$ is the vector of lower bounds, and @f$ u @f$ is the vector of upper bounds.
 *
 * If the problem is feasible, the solution vector @f$ x @f$ and the objective value are made available.
 * Otherwise, the Farekas ray  @f$ y @f$ is used to create the linear inequality @f$ (y^T A) x \le y^T b @f$,
 * which is infeasible over the local bounds.
 * In other words, even setting each element of @f$ x @f$ to the bound that minimise @f$ (y^A) x @f$,
 * its value is still greater than @f$ y^T b @f$.
 *
 * The usual workflow is as follows:
 * - For each real variable in the SMT problem, add a linked column with @ref AddColumn(const Variable&).
 * - For each symbolic formula in the SMT problem, add a linked row with @ref AddRow(const Variable&, const Formula&).
 * - Consolidate the LP problem with @ref Consolidate.
 * - Enable the active rows and bounds with @ref EnableRow and @ref EnableBound.
 * - Optimise the LP problem with @ref Optimise.
 *   - If the problem is feasible, update the model with @ref UpdateModelWithSolution.
 *   - If the problem is infeasible, generate an explanation using the infeasible rows and bounds.
 */
class LpSolver {
 public:
  using ColumnIndex = int;
  using RowIndex = int;
  static std::unique_ptr<LpSolver> GetInstance(const Config& config);

  /**
   * Construct a new LpSolver object with the given @p config .
   * @param config configuration to use
   * @param ninfinity negative infinity threshold value
   * @param infinity infinity threshold value
   * @param class_name name of the class
   */
  LpSolver(const Config& config, mpq_class ninfinity, mpq_class infinity, const std::string& class_name = "LpSolver");
  virtual ~LpSolver() = default;

  /** @getter{number of columns, lp solver} */
  [[nodiscard]] virtual int num_columns() const = 0;
  /** @getter{number of rows, lp solver} */
  [[nodiscard]] virtual int num_rows() const = 0;
  /** @getter{negative infinity threshold value, lp solver} */
  [[nodiscard]] const mpq_class& ninfinity() const { return ninfinity_; }
  /** @getter{infinity threshold value, lp solver} */
  [[nodiscard]] const mpq_class& infinity() const { return infinity_; }
  /** @getter{statistics, lp solver} */
  [[nodiscard]] const IterationStats& stats() const { return stats_; }
  /** @getter{configuration, lp solver} */
  [[nodiscard]] const Config& config() const { return config_; }
  /** @getter{primal solution\, if the lp is feasible\,, lp solver} */
  [[nodiscard]] const std::vector<mpq_class>& solution() const { return solution_; }
  /** @getter{dual solution\, if the lp is feasible\,, lp solver */
  [[nodiscard]] const std::vector<mpq_class>& dual_solution() const { return dual_solution_; }
  /** @getter{maps from and to SMT variables to LP columns/rows, lp solver} */
  [[nodiscard]] const std::unordered_map<Variable, int>& var_to_col() const { return var_to_col_; }
  /** @getter{maps from and to SMT variables to LP columns/rows, lp solver} */
  [[nodiscard]] const std::vector<Variable>& col_to_var() const { return col_to_var_; }
  /** @getter{vector of all the variables, lp solver} */
  [[nodiscard]] std::vector<Variable> variables() const;
  /**
   * Shorthand notation to get the real variable linked with column @p column.
   * @param column index of the column the real variable is linked to
   * @return corresponding real variable
   */
  [[nodiscard]] const Variable& var(const int column) const { return col_to_var_.at(column); }
  /**
   * Reserve space for the given number of columns and rows.
   *
   * Can speed up the addition of columns if the guess is close to the actual number.
   * @param size number of columns to reserve
   */
  virtual void ReserveColumns(int size);
  /**
   * Reserve space for the given number of rows.
   *
   * Can speed up the addition of rows if the guess is close to the actual number.
   * @param size number of rows to reserve
   */
  virtual void ReserveRows(int size);

  /**
   * Add a new unbounded column to the LP problem.
   * @pre The column must be added before the LP problem is consolidated.
   */
  ColumnIndex AddVariable(const Variable& var);
  /**
   * Add a new bounded column to the LP problem.
   * @pre The column must be added before the LP problem is consolidated.
   * @param obj coefficient of the variable in the objective function
   */
  ColumnIndex AddVariable(const Variable& var, const mpq_class& obj);
  /**
   * Add a new bounded column to the LP problem.
   * @pre The column must be added before the LP problem is consolidated.
   * @param lb lower bound of the column
   * @param ub upper bound of the column
   */
  ColumnIndex AddVariable(const Variable& var, const mpq_class& lb, const mpq_class& ub);
  /**
   * Add a new bounded column to the LP problem with a given @p obj coefficient.
   * @pre The column must be added before the LP problem is consolidated.
   * @param obj objective coefficient of the column
   * @param lb lower bound of the column
   * @param ub upper bound of the column
   */
  virtual ColumnIndex AddVariable(const Variable& var, const mpq_class& obj, const mpq_class& lb,
                                  const mpq_class& ub) = 0;

  /**
   * Add a new row to the LP problem with the given @p formula .
   * @pre All other rows already in the LP problem have been linked as well.
   * @param formula symbolic formula representing the row
   */
  RowIndex AddRow(const Formula& formula);
  /**
   * Add a new row to the LP problem with the given @p lhs expression, @p sense and @p rhs .
   *
   * The resulting row will be in the shape
   * @f[
   * lhs \text{ sense } rhs
   * @f]
   * where @f$ lhs @f$ is a linear expression, @f$ \text{ sense } \in \\{ \le, =, \ge \\} @f$
   * and @f$ rhs @f$ is a constant.
   * @param lhs expression on the left-hand side of the row
   * @param sense sense of the row (i.e. <=, =, >=)
   * @param rhs right-hand side of the row
   */
  RowIndex AddRow(const Expression& lhs, FormulaKind sense, const mpq_class& rhs);
  /**
   * Add a new row to the LP problem with the given @p lhs expression, @p sense and @p rhs .
   *
   * The resulting row will be in the shape
   * @f[
   * lhs \text{ sense } rhs
   * @f]
   * where @f$ lhs @f$ is a linear expression, @f$ \text{ sense } \in \\{ \le, =, \ge \\} @f$
   * and @f$ rhs @f$ is a constant.
   * @param lhs expression on the left-hand side of the row
   * @param sense sense of the row (i.e. <=, =, >=)
   * @param rhs right-hand side of the row
   */
  virtual RowIndex AddRow(const Expression::Addends& lhs, FormulaKind sense, const mpq_class& rhs) = 0;

  /**
   * Set the coefficient of the @p row constraint to apply at the @p column decisional variable.
   * @param row row of the constraint
   * @param column column containing the decisional variable to set the coefficient for
   * @param value new value of the coefficient
   */
  virtual void SetCoefficient(RowIndex row, ColumnIndex column, const mpq_class& value) = 0;

  /**
   * Set the objective coefficients of the LP problem to the given @p objective .
   * @param objective map from column index to objective coefficient
   */
  void SetObjective(const std::unordered_map<int, mpq_class>& objective);
  /**
   * Set the objective coefficients of the LP problem to the given @p objective .
   * @param objective
   */
  void SetObjective(const std::vector<mpq_class>& objective);
  /**
   * The the objective coefficient of the column corresponding to the give @p var to the given @p value .
   * @param var variable to set the objective for
   * @param value new objective coefficient for the column
   */
  void SetObjective(const Variable& var, const mpq_class& value);
  /**
   * The the objective coefficient of the given @p column to the given @p value .
   * @param column column to set the objective for
   * @param value new objective coefficient for the column
   */
  virtual void SetObjective(int column, const mpq_class& value) = 0;

  /**
   * Optimise the LP problem with the given @p precision .
   *
   * The result of the computation will be stored in @ref solution_ and @ref dual_solution_ if the problem is feasible,
   * and in @ref infeasible_rows_ and @ref infeasible_bounds_ otherwise.
   * If @p store_solution is false, the solution will not be stored, but the LpResult will still be returned.
   * The actual precision will be returned in the @p precision parameter.
   * @param[in,out] precision desired precision for the optimisation that becomes the actual precision achieved
   * @param store_solution whether the solution and dual solution should be stored
   * @return OPTIMAL if an optimal solution has been found and the return value of @p precision is @f$ = 0 @f$
   * @return DELTA_OPTIMAL if an optimal solution has been found and the return value of @p precision @f$\ge 0 @f$
   * @return UNBOUNDED if the problem is unbounded
   * @return INFEASIBLE if the problem is infeasible
   * @return ERROR if an error occurred
   */
  LpResult Optimise(mpq_class& precision, bool store_solution = true);

  virtual void PrintRow(std::ostream& os, int row) const = 0;
  virtual void PrintColumn(std::ostream& os, int column) const = 0;

#ifndef NDEBUG
  virtual void Dump() = 0;
#endif

 protected:
  virtual ColumnIndex AddVariableCore(const Variable& var, const mpq_class& lower_bound, const mpq_class& upper_bound,
                                      const mpq_class& obj) = 0;
  virtual RowIndex AddRowCore(const Expression& row, LpRowSense sense, const mpq_class& rhs) = 0;

  /**
   * Internal method that optimises the LP problem with the given @p precision .
   * @param precision desired precision for the optimisation
   * @param store_solution whether the solution and dual solution should be stored
   * @return OPTIMAL if an optimal solution has been found and the return value of @p precision is @f$ = 0 @f$
   * @return DELTA_OPTIMAL if an optimal solution has been found and the return value of @p precision @f$\ge 0 @f$
   * @return UNBOUNDED if the problem is unbounded
   * @return INFEASIBLE if the problem is infeasible
   * @return ERROR if an error occurred
   */
  virtual LpResult OptimiseCore(mpq_class& precision, bool store_solution) = 0;

  const Config& config_;  ///< Configuration to use
  IterationStats stats_;  ///< Statistics of the solver

  std::unordered_map<Variable, int> var_to_col_;  ///< Theory column ⇔ Variable.
                                                  ///< The column is the one used by the lp solver.
                                                  ///< The Variable is the one created by the PredicateAbstractor
  std::vector<Variable> col_to_var_;              ///< Literal ⇔ lp row.
                                                  ///< The literal is the one created by the PredicateAbstractor
                                                  ///< The row is the constraint used by the lp solver.
  std::vector<mpq_class> solution_;               ///< Solution vector
  std::vector<mpq_class> dual_solution_;          ///< Dual solution vector

  mpq_class ninfinity_;  ///< Negative infinity threshold value
  mpq_class infinity_;   ///< Infinity threshold value
};

std::ostream& operator<<(std::ostream& os, const LpSolver& solver);

}  // namespace delpi

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::LpSolver);

#endif
