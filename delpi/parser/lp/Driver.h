/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 * @file
 * Driver class.
 */
#pragma once

#include <map>
#include <optional>
#include <unordered_map>

#include "delpi/libs/gmp.h"
#include "delpi/parser/Driver.h"
#include "delpi/parser/lp/Column.h"
#include "delpi/parser/lp/OptType.h"
#include "delpi/parser/lp/Row.h"
#include "delpi/parser/lp/SenseType.h"

namespace delpi::lp {

class LpDriver final : public Driver {
 public:
  explicit LpDriver(LpSolver &lp_solver)
      : Driver(lp_solver, "LpDriver"),
        rows_{},
        columns_{},
        obj_{},
        objective_name_{"obj"},
        problem_name_{"lp"},
        is_min_{false} {}

  void MarkBinary(std::string_view var);
  void MarkGeneral(std::string_view var);
  void SetProblemName(std::string_view name);
  void SetObjectiveType(OptType opt);
  void SetObjectiveName(std::string_view name);
  void AddObjective(const std::unordered_map<std::string_view, mpq_class> &expr, std::string_view name = {});
  void AddConstraint(const std::unordered_map<std::string_view, mpq_class> &expr, SenseType sense, const mpq_class &rhs,
                     std::string_view name = {});
  void AddLowerBound(std::string_view var_name, const std::optional<mpq_class> &lb = std::nullopt);
  void AddUpperBound(std::string_view var_name, const std::optional<mpq_class> &ub = std::nullopt);
  void AddFreeBound(std::string_view);
  void End();

  const std::string &problem_name() const { return problem_name_; }
  const std::string &objective_name() const { return objective_name_; }
  bool is_min() const { return is_min_; }


 private:
  bool ParseStreamCore(std::istream &in) override;
  bool ParseFileCore(const std::string &filename) override;
  bool ParseStringCore(std::string_view input) override;

  Column &GetOrCreateColumn(std::string_view var_name);

  /**
   * The rows of the problem. Contains a map between each variable, stored as an expression, and the
   * coefficient. It will be used to build the final row_expression using the ExpressionAddFactory class.
   * The result is then combined with the rhs value and the correct row sense to build the Formula that makes up the
   * assertion.
   */
  std::map<std::string, Row, std::less<>> rows_;        ///< The rows of the problem.
  std::map<std::string, Column, std::less<>> columns_;  ///< The columns of the problem. Contains the variables.
  std::vector<std::pair<Variable, mpq_class>> obj_;     ///< The objective function.
  std::string objective_name_;                          ///< Name of the objective function
  std::string problem_name_;                            ///< Name of the problem
  bool is_min_;                                         ///< Whether we are dealing with a minimization problem
};

}  // namespace delpi::lp