/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * LpSolver class.
 */
#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "delpi/libs/gmp.h"
#include "delpi/solver/Column.h"
#include "delpi/solver/LpResult.h"
#include "delpi/solver/LpRowSense.h"
#include "delpi/solver/Row.h"
#include "delpi/symbolic/Expression.h"
#include "delpi/symbolic/Formula.h"
#include "delpi/symbolic/Variable.h"
#include "delpi/util/Config.h"
#include "delpi/util/Stats.h"
#include "delpi/util/concepts.h"

namespace delpi {

/**
 * Facade class that hides the underlying LP solver used by delpi.
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
 * - For each real variable in the SMT problem, add a linked column with @ref AddColumn
 * - For each symbolic formula in the SMT problem, add a linked row with @ref AddRow
 * - Optimise the LP problem with @ref Solve
 *   - If the problem is feasible, the solution is stored in @ref solution_ and @ref dual_solution_
 *   - If the problem is infeasible, the Farekas ray is stored in @ref dual_solution_
 */
class LpSolver {
 public:
  using ColumnIndex = int;
  using RowIndex = int;
  /**
   * Callback invoked by the LP solver when a solution (or delta solution) is found.
   * @param lp_solver LP solver that invoked the callback
   * @param result result of the LP solver
   * @param x solution vector
   * @param y dual solution vector
   * @param obj_lb lower bound of the objective
   * @param obj_ub upper bound of the objective
   * @param delta delta value
   * @return true if the solver should continue
   * @return false if the solver should stop
   */
  using SolveCallback = std::function<void(const LpSolver& lp_solver, LpResult result, const std::vector<mpq_class>& x,
                                           const std::vector<mpq_class>& y, const mpq_class& obj_lb,
                                           const mpq_class& obj_ub, const mpq_class& delta)>;
  /**
   * Callback invoked by the LP solver when a solution (or delta solution) is found.
   * @param lp_solver LP solver that invoked the callback
   * @param result result of the LP solver
   * @param x solution vector
   * @param y dual solution vector
   * @param obj_lb lower bound of the objective
   * @param obj_ub upper bound of the objective
   * @param diff difference between the lower and upper bounds
   * @param delta delta value
   * @return true if the solver should continue
   * @return false if the solver should stop
   */
  using PartialSolveCallback = std::function<bool(
      const LpSolver& lp_solver, LpResult result, const std::vector<mpq_class>& x, const std::vector<mpq_class>& y,
      const mpq_class& obj_lb, const mpq_class& obj_ub, const mpq_class& diff, const mpq_class& delta)>;

  static std::unique_ptr<LpSolver> GetInstance(const Config& config);

  /**
   * Construct a new LpSolver object with the given `config`.
   * @param ninfinity negative infinity threshold value
   * @param infinity infinity threshold value
   * @param config configuration to use
   * @param class_name name of the class
   */
  LpSolver(mpq_class ninfinity, mpq_class infinity, Config config = {}, const std::string& class_name = "LpSolver");
  virtual ~LpSolver() = default;

  /**
   * Parse the input file or stdin based on the Config parameters.
   * All variables, constraints, bounds and objective are added to the LP solver.
   * @return true if the parsing was successful
   * @return false if the parsing failed
   */
  bool Parse();
  /**
   * Parse the file with the given `filename`.
   * All variables, constraints, bounds and objective are added to the LP solver.
   * @param filename path to the file to parse
   * @return true if the parsing was successful
   * @return false if the parsing failed
   */
  bool ParseFile(const std::string& filename);
  /**
   * Parse the given `string` as input.
   * All variables, constraints, bounds and objective are added to the LP solver.
   * @param string string to parse
   * @return true if the parsing was successful
   * @return false if the parsing failed
   */
  bool ParseString(const std::string& string);
  /**
   * Parse the given `stream` as input.
   * All variables, constraints, bounds and objective are added to the LP solver.
   * @param stream stream to parse
   * @param stream_name name of the stream
   * @return true if the parsing was successful
   * @return false if the parsing failed
   */
  bool ParseStream(std::istream& stream, const std::string& stream_name = "(stdin)");

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
  /** @getter{dual solution\, if the lp is feasible\,, lp solver} */
  [[nodiscard]] const std::vector<mpq_class>& dual_solution() const { return dual_solution_; }
  /** @getter{maps from and to SMT variables to LP columns/rows, lp solver} */
  [[nodiscard]] const std::unordered_map<Variable, int>& var_to_col() const { return var_to_col_; }
  /** @getter{vector of all the variables, lp solver} */
  [[nodiscard]] const std::vector<Variable>& variables() const { return col_to_var_; }
  /** @getter{maps from and to LP columns to SMT variables, lp solver} */
  [[nodiscard]] std::vector<Formula> constraints() const;
  /** @getter{expected result collected from the file, lp problem} */
  [[nodiscard]] LpResult expected() const;
  /** @getter{mapping between each variable and its value, lp solution} */
  [[nodiscard]] std::unordered_map<Variable, mpq_class> model() const;
  /** @getter{callback function invoked upon solving the problem, lp solver} */
  [[nodiscard]] const SolveCallback& solve_cb() const { return solve_cb_; }
  /** @getsetter{callback function invoked upon solving the problem, lp solver} */
  [[nodiscard]] SolveCallback& m_solve_cb() { return solve_cb_; }
  /** @getter{callback function invoked upon finding a partial solution to the problem, lp solver} */
  [[nodiscard]] const PartialSolveCallback& partial_solve_cb() const { return partial_solve_cb_; }
  /** @getsetter{callback function invoked upon finding a partial solution to the problem, lp solver} */
  [[nodiscard]] PartialSolveCallback& m_partial_solve_cb() { return partial_solve_cb_; }
  /**
   * Get a mapping between the variables and their values in the solution vector `x`.
   * @param x solution vector
   * @return mapping between the variables and their values
   */
  [[nodiscard]] std::unordered_map<Variable, mpq_class> model(const std::vector<mpq_class>& x) const;
  /**
   * Get the value of `var` in the solution vector.
   * @param var variable to get the value for
   * @return solution value of the variable
   */
  [[nodiscard]] const mpq_class& solution(const Variable var) const { return solution_.at(var_to_col_.at(var)); }

  /**
   * Shorthand notation to get the real variable linked with column `column`.
   * @param column index of the column the real variable is linked to
   * @return corresponding real variable
   */
  [[nodiscard]] const Variable& var(const int column) const { return col_to_var_.at(column); }
  /**
   * Get the column at the given `column_idx` index.
   * @param column_idx index of the column to get
   * @return column structure
   */
  [[nodiscard]] virtual Column column(int column_idx) const = 0;
  /**
   * Get the row at the given `row_idx` index.
   * @param row_idx index of the row to get
   * @return row structure
   */
  [[nodiscard]] virtual Row row(int row_idx) const = 0;
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
   * Retrieve the information stored under the given `key`.
   * @param key key of the information to get
   * @return information stored under the given `key`
   */
  const std::string& GetInfo(const std::string& key) const;
  /**
   * Set the information stored under the given `key` to the given `value`.
   * @param key key of the information to set
   * @param value value to set the information to set
   */
  void SetInfo(const std::string& key, const std::string& value);
  /**
   * Set the option identified by the given `key` to the given `value`.
   *
   * The options will modify the Config of the LP solver.
   * Boolean parameters will be set to true if the value is "yes", "true", "1", or "on" (case-insensitive)
   * and false otherwise.
   * Available options are:
   *
   * - `:csv` (bool): whether to output in CSV format
   * - `:silent` (bool): whether to output nothing
   * - `:with-timings` (bool): whether to output timings
   * - `:precision` (double): precision of the solver
   * - `:continuous-output` (bool): whether to output continuously (at each delta-satisfying point)
   * - `:verbosity` (int): verbosity level
   * - `:simplex-verbosity` (int): verbosity level of the simplex solver
   * - `:produce-models` (bool): whether to produce models
   * - `:timeout` (int): timeout in milliseconds
   *
   * Note that the spacing before the '*' is mandatory.
   * ```
   *  * @set-option :precision 0.505
   *  * @set-option :produce-models false
   *  ENDATA
   *  ```
   * @note Parameters set from the command line will have precedence over the ones set in the input file.
   * @param key key of the option to set
   * @param value value of the option
   */
  void SetOption(const std::string& key, const std::string& value);

  /**
   * Add a new `column` to the LP problem.
   * Not indicating a `column.lb` or `column.ub` will result in an unbounded variable in that direction.
   * @warning The objective coefficient is set with respect to a minimisation problem.
   * @param column column to add to the LP problem
   * @return index of the last column added
   */
  ColumnIndex AddColumn(const Column& column);
  /**
   * Add a new unbounded column corresponding to the variable `var` to the LP problem.
   * @param var variable to add to the LP problem
   * @return index of the last column added
   */
  ColumnIndex AddColumn(const Variable& var);
  /**
   * Add a new column to the LP problem setting the objective coefficient of `var` to the given `obj`.
   * @warning The objective coefficient is set with respect to a minimisation problem.
   * @param var variable to add to the LP problem
   * @param obj coefficient of the variable in the objective function for minimisation
   * @return index of the last column added
   */
  ColumnIndex AddColumn(const Variable& var, const mpq_class& obj);
  /**
   * Add a new bounded column to the LP problem, ensuring that the variable `var` is in the range @f$ [lb, ub] @f$.
   * @param var variable to add to the LP problem
   * @param lb lower bound of the column
   * @param ub upper bound of the column
   * @return index of the last column added
   */
  ColumnIndex AddColumn(const Variable& var, const mpq_class& lb, const mpq_class& ub);
  /**
   * Add a new bounded column to the LP problem, ensuring that the variable `var` is in the range @f$ [lb, ub] @f$ and
   * has the objective coefficient `obj`.
   * @warning The objective coefficient is set with respect to a minimisation problem.
   * @param var variable to add to the LP problem
   * @param obj objective coefficient of the column
   * @param lb lower bound of the column
   * @param ub upper bound of the column
   * @return index of the last column added
   */
  virtual ColumnIndex AddColumn(const Variable& var, const mpq_class& obj, const mpq_class& lb,
                                const mpq_class& ub) = 0;

  /**
   * Add a new row to the LP problem with the given `row`.
   * Not indicating a `row.lb` or `row.ub` will result in an unbounded row in that direction.
   * If `row.lb` and `row.ub` are equal, a single equality constraint is added.
   * Otherwise, a pair of inequality constraints may be added, depending on the underlying solver implementation.
   * @param row structure of the row to add
   * @return index of the last row added
   */
  RowIndex AddRow(const Row& row);
  /**
   * Add a new row to the LP problem with the given `addends` bounded by `lb` and `ub`.
   * If `lb` and `ub` are equal, a single equality constraint is added.
   * Otherwise, a pair of inequality constraints may be added, depending on the underlying solver implementation.
   * @param addends vector of pairs (Variable, coeff) that represent the linear summation of the row
   * @param lb lower bound of the row
   * @param ub upper bound of the row
   * @return index of the last row added
   */
  virtual RowIndex AddRow(const std::vector<std::pair<Variable, mpq_class>>& addends, const mpq_class& lb,
                          const mpq_class& ub) = 0;
  /**
   * Add a new row to the LP problem with the given `formula`.
   * @param formula symbolic formula representing a constraint to add as a row
   */
  RowIndex AddRow(const Formula& formula);
  /**
   * Add a new row to the LP problem with the given `lhs` linear expression, `sense` and `rhs`.
   *
   * The resulting row will be in the shape
   * @f[
   * lhs \text{ sense } rhs
   * @f]
   * where @f$ lhs @f$ is a linear expression, @f$ \text{ sense } \in \\{ \le, =, \ge \\} @f$
   * and @f$ rhs @f$ is a constant.
   * @param lhs linear expression on the left-hand side of the row
   * @param sense sense of the row (i.e. @f$ \le, =, \ge @f$)
   * @param rhs constant on the right-hand side of the row
   */
  RowIndex AddRow(const Expression& lhs, FormulaKind sense, const mpq_class& rhs);
  /**
   * Add a new row to the LP problem with the given `lhs` linear summation, `sense` and `rhs`.
   *
   * The resulting row will be in the shape
   * @f[
   * lhs \text{ sense } rhs
   * @f]
   * where @f$ lhs @f$ is a linear expression, @f$ \text{ sense } \in \\{ \le, =, \ge \\} @f$
   * and @f$ rhs @f$ is a constant.
   * @param lhs linear summation on the left-hand side of the row
   * @param sense sense of the row (i.e. <=, =, >=)
   * @param rhs right-hand side of the row
   */
  virtual RowIndex AddRow(const Expression::Addends& lhs, FormulaKind sense, const mpq_class& rhs) = 0;

  /**
   * Set the coefficient of the `row` constraint to apply at the `column` decisional variable.
   * @param row row of the constraint
   * @param column column containing the decisional variable to set the coefficient for
   * @param value new value of the coefficient
   */
  virtual void SetCoefficient(RowIndex row, ColumnIndex column, const mpq_class& value) = 0;

  /**
   * Set the objective coefficients of the LP problem to the given `objective`.
   * @param objective map from column index to objective coefficient
   */
  void SetObjective(const std::unordered_map<int, mpq_class>& objective);
  /**
   * Set the objective coefficients of the LP problem to the given `objective`.
   * @param objective
   */
  void SetObjective(const std::vector<mpq_class>& objective);
  /**
   * The the objective coefficient of the column corresponding to the give `var` to the given `value`.
   * @param var variable to set the objective for
   * @param value new objective coefficient for the column
   */
  void SetObjective(const Variable& var, const mpq_class& value);
  /**
   * The the objective coefficient of the given `column` to the given `value`.
   * @param column column to set the objective for
   * @param value new objective coefficient for the column
   */
  virtual void SetObjective(int column, const mpq_class& value) = 0;

  /**
   * Set the bounds of a `var` in the LP problem to the given `lb` and `ub`.
   * @param var variable to be bounded
   * @param lb lower bound
   * @param ub upper bound
   */
  virtual void SetBound(Variable var, const mpq_class& lb, const mpq_class& ub) = 0;

  /**
   * Optimise the LP problem with the given `precision`.
   *
   * The result of the computation will be stored in @ref solution_ and @ref dual_solution_ if the problem is feasible.
   * If `store_solution` is false, the solution will not be stored, but the LpResult will still be returned.
   * The actual precision will be returned in the `precision` parameter.
   * @param[in,out] precision desired precision for the optimisation that becomes the actual precision achieved
   * @param store_solution whether the solution and dual solution should be stored
   * @return OPTIMAL if an optimal solution has been found and the return value of `precision` is @f$ = 0 @f$
   * @return DELTA_OPTIMAL if an optimal solution has been found and the return value of `precision` @f$\ge 0 @f$
   * @return UNBOUNDED if the problem is unbounded
   * @return INFEASIBLE if the problem is infeasible
   * @return ERROR if an error occurred
   */
  LpResult Solve(mpq_class& precision, bool store_solution = true);

  /**
   * Set the `objective_function` to maximise while being subject to all the constraints.
   * The objective function coefficients will overwrite the current ones, if any.
   * @warning The objective coefficient of variables not appearing in the `objective_function` is not altered.
   * @param objective_function expression to maximise}
   */
  void Maximise(const Expression& objective_function);
  /**
   * Set the `objective_function` to maximise while being subject to all the constraints.
   * The objective function coefficients will overwrite the current ones, if any.
   * @tparam T iterable of pairs (Variable, mpq_class) representing the objective function
   * @warning The objective coefficient of variables not appearing in the `objective_function` is not altered.
   * @param objective_function expression to maximise}
   */
  template <TypedIterable<std::pair<const Variable, mpq_class>> T>
  void Maximise(const T& objective_function);
  /**
   * Set the `objective_function` to minimise while being subject to all the constraints.
   * The objective function coefficients will overwrite the current ones, if any.
   * @warning The objective coefficient of variables not appearing in the `objective_function` is not altered.
   * @param objective_function expression to minimise
   */
  void Minimise(const Expression& objective_function);
  /**
   * Set the `objective_function` to maximise while being subject to all the constraints.
   * The objective function coefficients will overwrite the current ones, if any.
   * @tparam T iterable of pairs (Variable, mpq_class) representing the objective function
   * @warning The objective coefficient of variables not appearing in the `objective_function` is not altered.
   * @param objective_function expression to maximise}
   */
  template <TypedIterable<std::pair<const Variable, mpq_class>> T>
  void Minimise(const T& objective_function);

  /**
   * Check whether the result obtained by the solver is compatible with the one collected from the file.
   * @param result result obtained by the solver
   * @return true if the result obtained by the solver is compatible with the one collected from the file
   * @return false if the result obtained by the solver is incompatible with the one collected from the file
   */
  [[nodiscard]] bool CheckAgainstExpected(LpResult result) const;

  /**
   * Verify that the current @ref solution_ satisfies all the constraints in the LpSolver.
   * @return true if the current @ref solution_ verifies all the constraints
   * @return false if at least one constraint is violated by the current @ref solution_.
   */
  bool Verify() const;

#ifndef NDEBUG
  virtual void Dump() = 0;
#endif

 protected:
  /**
   * Internal method that optimises the LP problem with the given `precision`.
   * @param precision desired precision for the optimisation
   * @param store_solution whether the solution and dual solution should be stored
   * @return OPTIMAL if an optimal solution has been found and the return value of `precision` is @f$ = 0 @f$
   * @return DELTA_OPTIMAL if an optimal solution has been found and the return value of `precision` @f$\ge 0 @f$
   * @return UNBOUNDED if the problem is unbounded
   * @return INFEASIBLE if the problem is infeasible
   * @return ERROR if an error occurred
   */
  virtual LpResult SolveCore(mpq_class& precision, bool store_solution) = 0;

  /**
   * Check whether the row that is about to be added is a simple bound.
   * If that is the case, the LP solver should add a simple bound instead of a row.
   * A simple bound is a constraint of the form @f$ l \le c \cdot x \le u @f$,
   * where @f$ l @f$ and @f$ u @f$ are the lower and upper bounds of the variable @f$ x @f$ and @f$ c @f$ is a constant.
   * Any other constraint should be added as a proper row.
   * @param addends linear summation of the row. If it only contains a single variable, it is a simple bound
   * @param lb lower bound of the row
   * @param ub upper bound of the row
   * @return true if the LP solver has added a simple bound instead of a row
   * @return false if the LP solver should add proper row
   */
  bool SetSimpleBoundInsteadOfAddRow(const std::vector<Expression::Addend>& addends, const mpq_class& lb,
                                     const mpq_class& ub);

  Config config_;                                      ///< Configuration to use
  IterationStats stats_;                               ///< Statistics of the solver
  std::unordered_map<std::string, std::string> info_;  ///< Generic information map. Generally collected from the file

  std::unordered_map<Variable, int> var_to_col_;  ///< Theory column ⇔ Variable.
                                                  ///< The column is the one used by the lp solver.
                                                  ///< The Variable is the one created by the PredicateAbstractor
  std::vector<Variable> col_to_var_;              ///< Literal ⇔ lp row.
                                                  ///< The literal is the one created by the PredicateAbstractor
                                                  ///< The row is the constraint used by the lp solver.
  std::vector<mpq_class> solution_;               ///< Solution vector
  std::vector<mpq_class> dual_solution_;          ///< Dual solution vector
  mpq_class obj_lb_;                              ///< Lower bound on the objective value, if any
  mpq_class obj_ub_;                              ///< Upper bound on the objective value, if any

  SolveCallback solve_cb_;                 ///< Callback to call after solving the LP problem
  PartialSolveCallback partial_solve_cb_;  ///< Callback to call after solving the LP problem with a partial solution

  mpq_class ninfinity_;  ///< Negative infinity threshold value
  mpq_class infinity_;   ///< Infinity threshold value
};

std::ostream& operator<<(std::ostream& os, const LpSolver& solver);

}  // namespace delpi

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::LpSolver);

#endif
