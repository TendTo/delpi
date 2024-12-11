/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "LpSolver.h"

#include <ostream>
#include <utility>

#if DELPI_ENABLED_QSOPTEX
#include "delpi/solver/QsoptexLpSolver.h"
#endif
#if DELPI_ENABLED_SOPLEX
#include "delpi/solver/SoplexLpSolver.h"
#endif
#include "delpi/util/error.h"

namespace delpi {

LpSolver::LpSolver(const Config& config, mpq_class ninfinity, mpq_class infinity, const std::string& class_name)
    : config_{config},
      stats_{config.with_timings(), class_name, "Total time spent in Optimise", "Total # of Optimise"},
      var_to_col_{},
      col_to_var_{},
      solution_{},
      dual_solution_{},
      ninfinity_{std::move(ninfinity)},
      infinity_{std::move(infinity)} {}

std::unique_ptr<LpSolver> LpSolver::GetInstance(const Config& config) {
  switch (config.lp_solver()) {
    case Config::LPSolver::SOPLEX:
      return std::make_unique<SoplexLpSolver>(config);
    case Config::LPSolver::QSOPTEX:
      return std::make_unique<QsoptexLpSolver>(config);
    default:
      DELPI_UNREACHABLE();
  }
}

LpSolver::ColumnIndex LpSolver::AddColumn(const Column& column) {
  DELPI_ASSERT(!var_to_col_.contains(column.var), "Variable already exists in the LP.");
  return AddColumn(column.var, column.obj.value_or(ninfinity_), column.lb.value_or(infinity_), column.ub.value_or(0));
}
LpSolver::ColumnIndex LpSolver::AddColumn(const Variable& var) {
  DELPI_ASSERT(!var_to_col_.contains(var), "Variable already exists in the LP.");
  // Add a column representing this variable to the lp solver
  return AddColumn(var, 0, 0, infinity_);
}
LpSolver::ColumnIndex LpSolver::AddColumn(const Variable& var, const mpq_class& obj) {
  DELPI_ASSERT(!var_to_col_.contains(var), "Variable already exists in the LP.");
  // Add a column representing this variable to the lp solver
  return AddColumn(var, obj, 0, infinity_);
}
LpSolver::ColumnIndex LpSolver::AddColumn(const Variable& var, const mpq_class& lb, const mpq_class& ub) {
  DELPI_ASSERT(!var_to_col_.contains(var), "Variable already exists in the LP.");
  // Add a column representing this variable to the lp solver
  return AddColumn(var, 0, lb, ub);
}

LpSolver::RowIndex LpSolver::AddRow(const Formula& formula) {
  return AddRow(formula.expression(), formula.kind(), formula.rhs());
}
LpSolver::RowIndex LpSolver::AddRow(const Expression& lhs, const FormulaKind sense, const mpq_class& rhs) {
  return AddRow(lhs.addends(), sense, rhs);
}

void LpSolver::ReserveColumns(const int size) { DELPI_ASSERT(size >= 0, "Invalid number of columns."); }
void LpSolver::ReserveRows(const int size) { DELPI_ASSERT(size >= 0, "Invalid number of rows."); }

void LpSolver::SetObjective(const std::unordered_map<int, mpq_class>& objective) {
  for (const auto& [column, value] : objective) SetObjective(column, value);
}
void LpSolver::SetObjective(const std::vector<mpq_class>& objective) {
  for (int i = 0; i < static_cast<int>(objective.size()); ++i) SetObjective(i, objective.at(i));
}
LpResult LpSolver::Optimise(mpq_class& precision, const bool store_solution) {
  DELPI_ASSERT(num_rows() > 0, "Cannot optimise without rows.");
  DELPI_ASSERT(num_columns() > 0, "Cannot optimise without columns.");
  const TimerGuard timer_guard(&stats_.m_timer(), stats_.enabled());
  stats_.Increase();
  solution_.clear();
  dual_solution_.clear();
  return OptimiseCore(precision, store_solution);
}
void LpSolver::SetObjective(const Variable& var, const mpq_class& value) { SetObjective(var_to_col_.at(var), value); }

std::ostream& operator<<(std::ostream& os, const LpSolver& solver) {
  os << solver.stats().class_name() << " {";
  os << "num_columns: " << solver.num_columns() << ", ";
  os << "num_rows: " << solver.num_rows() << ", ";
  os << "ninfinity: " << solver.ninfinity() << ", ";
  os << "infinity: " << solver.infinity() << ", ";
  os << "stats: " << solver.stats() << ", ";
  os << "config: " << solver.config() << ", ";
  if (!solver.solution().empty()) {
    os << "solution: ";
    for (int i = 0; i < static_cast<int>(solver.solution().size()); ++i) {
      os << solver.col_to_var().at(i) << " = " << solver.solution().at(i) << ", ";
    }
  }
  os << "}";
  return os;
}

}  // namespace delpi
