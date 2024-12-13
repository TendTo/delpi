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
#include <span>
#include <unordered_set>

#include "delpi/util/error.h"

namespace delpi {

namespace {
bool IsYes(std::string value) {
  std::ranges::transform(value, value.begin(), [](const unsigned char c) { return std::tolower(c); });
  return value == "yes" || value == "true" || value == "1" || value == "on";
}
}  // namespace

LpSolver::LpSolver(mpq_class ninfinity, mpq_class infinity, Config config, const std::string& class_name)
    : config_{std::move(config)},
      stats_{config.with_timings(), class_name, "Total time spent in Optimise", "Total # of Optimise"},
      var_to_col_{},
      col_to_var_{},
      solution_{},
      dual_solution_{},
      solve_cb_{},
      ninfinity_{std::move(ninfinity)},
      infinity_{std::move(infinity)} {}

std::unique_ptr<LpSolver> LpSolver::GetInstance(const Config& config) {
  switch (config.lp_solver()) {
    case Config::LpSolver::SOPLEX:
      return std::make_unique<SoplexLpSolver>(config);
    case Config::LpSolver::QSOPTEX:
      return std::make_unique<QsoptexLpSolver>(config);
    default:
      DELPI_UNREACHABLE();
  }
}

LpResult LpSolver::expected() const {
  if (!info_.contains(":status")) return LpResult::UNSOLVED;
  const std::string& expected = info_.at(":status");
  if (expected == "optimal") return LpResult::OPTIMAL;
  if (expected == "delta-optimal") return LpResult::DELTA_OPTIMAL;
  if (expected == "infeasible") return LpResult::INFEASIBLE;
  if (expected == "unbounded") return LpResult::UNBOUNDED;
  if (expected == "error") return LpResult::ERROR;
  return LpResult::UNSOLVED;
}

std::unordered_map<Variable, mpq_class> LpSolver::model() const { return model(solution_); }
std::unordered_map<Variable, mpq_class> LpSolver::model(const std::vector<mpq_class>& x) const {
  if (x.empty()) return {};
  DELPI_ASSERT(col_to_var_.size() == x.size(), "All variables must appear in the solution");
  std::unordered_map<Variable, mpq_class> model;
  model.reserve(col_to_var_.size());
  for (std::size_t i = 0; i < col_to_var_.size(); ++i) model.emplace(col_to_var_.at(i), x.at(i));
  return model;
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

LpSolver::RowIndex LpSolver::AddRow(const Row& row) {
  return AddRow(row.addends, row.lb.value_or(ninfinity_), row.ub.value_or(infinity_));
}
LpSolver::RowIndex LpSolver::AddRow(const Formula& formula) {
  return AddRow(formula.expression(), formula.kind(), formula.rhs());
}
LpSolver::RowIndex LpSolver::AddRow(const Expression& lhs, const FormulaKind sense, const mpq_class& rhs) {
  return AddRow(lhs.addends(), sense, rhs);
}

std::vector<Formula> LpSolver::constraints() const {
  std::vector<Formula> constraints;
  constraints.reserve(num_rows() + num_columns());
  for (int i = 0; i < num_rows(); ++i) {
    const auto [addends, lb, ub] = row(i);
    if (lb.has_value() && ub.has_value()) {
      if (lb.value() == ub.value()) {
        constraints.emplace_back(Expression{addends}, FormulaKind::Eq, ub.value());
      } else {
        constraints.emplace_back(Expression{addends}, FormulaKind::Leq, ub.value());
        constraints.emplace_back(Expression{addends}, FormulaKind::Geq, lb.value());
      }
    } else if (lb.has_value()) {
      constraints.emplace_back(Expression{addends}, FormulaKind::Geq, lb.value());
    } else if (ub.has_value()) {
      constraints.emplace_back(Expression{addends}, FormulaKind::Leq, ub.value());
    }
  }
  for (int i = 0; i < num_columns(); ++i) {
    const auto [var, lb, ub, obj] = column(i);
    if (lb.has_value() && ub.has_value()) {
      if (lb.value() == ub.value()) {
        constraints.emplace_back(Expression{var}, FormulaKind::Eq, ub.value());
      } else {
        constraints.emplace_back(Expression{var}, FormulaKind::Leq, ub.value());
        constraints.emplace_back(Expression{var}, FormulaKind::Geq, lb.value());
      }
    } else if (lb.has_value()) {
      constraints.emplace_back(Expression{var}, FormulaKind::Geq, lb.value());
    } else if (ub.has_value()) {
      constraints.emplace_back(Expression{var}, FormulaKind::Leq, ub.value());
    }
  }
  return constraints;
}
void LpSolver::ReserveColumns([[maybe_unused]] const int size) {
  DELPI_ASSERT(size >= 0, "Invalid number of columns.");
}
void LpSolver::ReserveRows([[maybe_unused]] const int size) { DELPI_ASSERT(size >= 0, "Invalid number of rows."); }

const std::string& LpSolver::GetInfo(const std::string& key) const { return info_.at(key); }
void LpSolver::SetInfo(const std::string& key, const std::string& value) { info_.emplace(key, value); }
void LpSolver::SetOption(const std::string& key, const std::string& value) {
  DELPI_TRACE_FMT("LpSolver::SetOption({}, {})", key, value);
  if (key == ":csv") {
    config_.m_csv().SetFromFile(IsYes(value));
  } else if (key == ":silent") {
    config_.m_silent().SetFromFile(IsYes(value));
  } else if (key == ":with-timings") {
    config_.m_with_timings().SetFromFile(IsYes(value));
  } else if (key == ":precision") {
    config_.m_precision().SetFromFile(std::stod(value));
  } else if (key == ":continuous-output") {
    config_.m_continuous_output().SetFromFile(IsYes(value));
  } else if (key == ":verbosity") {
    config_.m_verbose_delpi().SetFromFile(std::stoi(value));
  } else if (key == ":simplex-verbosity") {
    config_.m_verbose_simplex().SetFromFile(std::stoi(value));
  } else if (key == ":produce-models") {
    config_.m_produce_models().SetFromFile(IsYes(value));
  } else if (key == ":timeout") {
    config_.m_timeout().SetFromFile(std::stoi(value));
  } else {
    DELPI_ERROR_FMT("Unknown option: {} = {}. Ignored", key, value);
  }
}

void LpSolver::SetObjective(const std::unordered_map<int, mpq_class>& objective) {
  for (const auto& [column, value] : objective) SetObjective(column, value);
}
void LpSolver::SetObjective(const std::vector<mpq_class>& objective) {
  for (int i = 0; i < static_cast<int>(objective.size()); ++i) SetObjective(i, objective.at(i));
}
LpResult LpSolver::Solve(mpq_class& precision, const bool store_solution) {
  DELPI_ASSERT(num_rows() > 0, "Cannot optimise without rows.");
  DELPI_ASSERT(num_columns() > 0, "Cannot optimise without columns.");
  DELPI_DEBUG_FMT("LpSolver::Solve({}, {})", precision, store_solution);
  const TimerGuard timer_guard(&stats_.m_timer(), stats_.enabled());
  stats_.Increase();
  solution_.clear();
  dual_solution_.clear();
  const LpResult result = SolveCore(precision, store_solution);
  if (solve_cb_) solve_cb_(*this, result, solution_, dual_solution_, obj_lb_, obj_ub_, precision);
  return result;
}
void LpSolver::SetObjective(const Variable& var, const mpq_class& value) { SetObjective(var_to_col_.at(var), value); }

void LpSolver::Maximise(const Expression& objective_function) { Maximise(objective_function.addends()); }
template <TypedIterable<std::pair<const Variable, mpq_class>> T>
void LpSolver::Maximise(const T& objective_function) {
  DELPI_TRACE_FMT("LpSolver::Maximise({})", objective_function);
  for (const auto& [var, coeff] : objective_function) SetObjective(var, -coeff);
}
void LpSolver::Minimise(const Expression& objective_function) { Minimise(objective_function.addends()); }
template <TypedIterable<std::pair<const Variable, mpq_class>> T>
void LpSolver::Minimise(const T& objective_function) {
  DELPI_TRACE_FMT("LpSolver::Minimise({})", objective_function);
  for (const auto& [var, coeff] : objective_function) SetObjective(var, coeff);
}

bool LpSolver::ConflictingExpected(const LpResult result) const {
  DELPI_TRACE_FMT("LpSolver::ConflictingExpected({})", result);
  switch (expected()) {
    case LpResult::OPTIMAL:
      return result == LpResult::OPTIMAL || result == LpResult::DELTA_OPTIMAL || result == LpResult::UNBOUNDED;
    case LpResult::DELTA_OPTIMAL:
      return result == LpResult::DELTA_OPTIMAL;
    case LpResult::UNBOUNDED:
      return result == LpResult::UNBOUNDED;
    default:
      return false;
  }
}

bool LpSolver::Verify() {
  DELPI_TRACE("LpSolver::Verify()");
  return true;
}

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
      os << solver.variables().at(i) << " = " << solver.solution().at(i) << ", ";
    }
  }
  os << "}";
  return os;
}

template void LpSolver::Maximise(const std::vector<std::pair<Variable, mpq_class>>&);
template void LpSolver::Maximise(const std::set<std::pair<Variable, mpq_class>>&);
template void LpSolver::Maximise(const std::unordered_set<std::pair<Variable, mpq_class>>&);
template void LpSolver::Maximise(const std::span<std::pair<Variable, mpq_class>>&);
template void LpSolver::Maximise(const std::map<Variable, mpq_class>&);
template void LpSolver::Maximise(const std::unordered_map<Variable, mpq_class>&);

template void LpSolver::Minimise(const std::vector<std::pair<Variable, mpq_class>>&);
template void LpSolver::Minimise(const std::set<std::pair<Variable, mpq_class>>&);
template void LpSolver::Minimise(const std::unordered_set<std::pair<Variable, mpq_class>>&);
template void LpSolver::Minimise(const std::span<std::pair<Variable, mpq_class>>&);
template void LpSolver::Minimise(const std::map<Variable, mpq_class>&);

}  // namespace delpi
