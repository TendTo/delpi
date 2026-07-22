/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 * @file
 * Driver class.
 */
#include "delpi/parser/lp/Driver.h"

#include "Scanner.h"
#include "delpi/util/logging.h"

namespace delpi::lp {

void LpDriver::MarkBinary(const std::string_view var) {
  DELPI_TRACE_FMT("({})", var);
  Column& col = GetOrCreateColumn(var);
  col.is_binary = true;
}

void LpDriver::MarkGeneral(const std::string_view var) {
  DELPI_TRACE_FMT("({})", var);
  GetOrCreateColumn(var);
}

void LpDriver::SetProblemName(const std::string_view name) {
  DELPI_TRACE_FMT("({})", name);
  problem_name_ = name;
}

void LpDriver::SetObjectiveType(const OptType opt) {
  DELPI_TRACE_FMT("({})", opt);
  is_min_ = opt == OptType::MIN;
}

void LpDriver::SetObjectiveName(const std::string_view name) {
  DELPI_TRACE_FMT("({})", name);
  objective_name_ = name;
}

void LpDriver::AddObjective(const std::unordered_map<std::string_view, mpq_class>& expr, const std::string_view name) {
  DELPI_TRACE_FMT("({}, {})", expr, name);
  for (const auto& [var_name, coeff] : expr) {
    const Variable& var = GetOrCreateColumn(var_name).var;
    obj_.emplace_back(var, coeff);
  }
  objective_name_ = name.empty() ? "obj" : std::string{name};
}
void LpDriver::AddConstraint(const std::unordered_map<std::string_view, mpq_class>& expr, const SenseType sense,
                             const mpq_class& rhs, [[maybe_unused]] std::string_view name) {
  DELPI_TRACE_FMT("({}, {}, {}, {})", expr, sense, rhs, name);
  Row row;
  row.addends.reserve(expr.size());
  for (const auto& [var_name, coeff] : expr) {
    const Variable& var = GetOrCreateColumn(var_name).var;
    row.addends.emplace_back(var, coeff);
  }
  if (sense == SenseType::L || sense == SenseType::E) row.ub = rhs;
  if (sense == SenseType::G || sense == SenseType::E) row.lb = rhs;
  rows_.emplace(name.empty() ? std::to_string(rows_.size()) : std::string{name}, row);
}

void LpDriver::AddLowerBound(const std::string_view var_name, const std::optional<mpq_class>& lb) {
  DELPI_TRACE_FMT("({}, {})", var_name, lb.has_value() ? lb->get_str() : "-inf");
  GetOrCreateColumn(var_name).lb = lb.has_value() ? lb : lp_solver_.ninfinity();
}
void LpDriver::AddUpperBound(const std::string_view var_name, const std::optional<mpq_class>& ub) {
  DELPI_TRACE_FMT("({}, {})", var_name, ub.has_value() ? ub->get_str() : "inf");
  GetOrCreateColumn(var_name).ub = ub.has_value() ? ub : lp_solver_.infinity();
}
void LpDriver::AddFreeBound(const std::string_view var_name) {
  DELPI_TRACE_FMT("({})", var_name);
  Column& col = GetOrCreateColumn(var_name);
  col.lb = lp_solver_.ninfinity();
  col.ub = lp_solver_.infinity();
}

void LpDriver::End() {
  DELPI_DEBUG_FMT("Driver::EndData reached end of file {}", problem_name_);
  DELPI_DEBUG_FMT("Found {} variables and {} constraints", columns_.size(), rows_.size());
  static const mpq_class zero{0};
  static const mpq_class one{1};

  lp_solver_.ReserveColumns(columns_.size());
  for (const auto& [name, column_data] : columns_) {
    // The lower bound is either
    // - set explicitly
    // - 0 if the upper bound is greater than 0
    // - negative infinity if the upper bound is less than 0
    const mpq_class& lb = column_data.lb.has_value()                                 ? column_data.lb.value()
                          : column_data.ub.has_value() && column_data.ub.value() < 0 ? lp_solver_.ninfinity()
                                                                                     : zero;
    // The upper bound is either
    // - set explicitly
    // - 1 if no explicit upper bound has been set and the variable is binary
    // - positive infinity if no explicit upper bound has been set and the variable is not an integer
    const mpq_class& ub = column_data.ub.has_value() ? column_data.ub.value()
                          : column_data.is_binary    ? one
                                                     : lp_solver_.infinity();
    lp_solver_.AddColumn(column_data.var, lb, ub);
  }

  lp_solver_.ReserveRows(rows_.size());
  for (const auto& [row, row_data] : rows_) {
    if (row_data.addends.empty()) continue;  // No point in adding empty rows
    lp_solver_.AddRow(row_data.addends, row_data.lb.value_or(lp_solver_.ninfinity()),
                      row_data.ub.value_or(lp_solver_.infinity()));
  }

  if (is_min_) {
    lp_solver_.Minimise(obj_);
  } else {
    lp_solver_.Maximise(obj_);
  }
}

bool LpDriver::ParseStreamCore(std::istream& in) {
  // istream to string
  LpScanner scanner(*this);
  const std::string input{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
  return scanner.ParseString(input);
}

bool LpDriver::ParseFileCore(const std::string& filename) {
  LpScanner scanner(*this);
  return scanner.ParseFile(filename);
}

bool LpDriver::ParseStringCore(const std::string_view input) {
  LpScanner scanner(*this);
  return scanner.ParseString(input);
}

Column& LpDriver::GetOrCreateColumn(const std::string_view var_name) {
  const auto it = columns_.find(var_name);
  return it == columns_.end() ? columns_.emplace(var_name, Column{Variable{std::string{var_name}}}).first->second
                              : it->second;
}

}  // namespace delpi::lp