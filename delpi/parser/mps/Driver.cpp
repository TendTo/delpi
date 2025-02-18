/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */

#include "delpi/parser/mps/Driver.h"

#include <ostream>

#include "delpi/util/error.h"
#include "delpi/util/logging.h"

namespace delpi::mps {

MpsDriver::MpsDriver(LpSolver &lp_solver) : Driver{lp_solver, "MpsDriver"} {}

bool MpsDriver::ParseStreamCore(std::istream &in) {
  MpsScanner scanner(&in);
  scanner.set_debug(lp_solver_.config().debug_scanning());
  scanner_ = &scanner;

  MpsParser parser(*this);
  parser.set_debug_level(lp_solver_.config().debug_parsing());
  const bool res = parser.parse() == 0;
  scanner_ = nullptr;
  return res;
}

bool MpsDriver::VerifyStrictBound(const std::string &bound) {
  if (strict_mps_) {
    if (bound_name_.empty()) {
      bound_name_ = bound;
    } else if (bound_name_ != bound) {
      DELPI_WARN_FMT("First bound was '{}', found new bound '{}'. Skipping", bound_name_, bound);
      return false;
    }
  }
  return true;
}

bool MpsDriver::VerifyStrictRhs(const std::string &rhs) {
  if (strict_mps_) {
    if (rhs_name_.empty()) {
      rhs_name_ = rhs;
    } else if (rhs_name_ != rhs) {
      DELPI_WARN_FMT("First RHS was '{}', found new RHS '{}'. Skipping", rhs_name_, rhs);
      return false;
    }
  }
  return true;
}

void MpsDriver::error(const location &l, const std::string &m) { std::cerr << l << " : " << m << std::endl; }

void MpsDriver::ObjectiveSense(bool is_min) {
  DELPI_TRACE_FMT("Driver::ObjectiveSense {}", is_min);
  is_min_ = is_min;
}

void MpsDriver::ObjectiveName(const std::string &row) {
  DELPI_TRACE_FMT("Driver::ObjectiveName {}", row);
  obj_row_ = row;
}

void MpsDriver::AddRow(const SenseType sense, const std::string &row) {
  DELPI_TRACE_FMT("Driver::AddRow {} {}", sense, row);
  if (sense == SenseType::N && obj_row_.empty()) {
    DELPI_DEBUG("Objective row name not found. Adding the first row with sense N as objective row");
    obj_row_ = row;
    return;
  }
  rows_.emplace(row, Row{sense});
}

void MpsDriver::AddColumn(const std::string &column, const std::string &row, mpq_class value) {
  DELPI_TRACE_FMT("Driver::AddColumn {} {} {}", row, column, value);
  auto it = columns_.find(column);
  if (columns_.end() == it) {
    DELPI_TRACE_FMT("Added column {}", column);
    // Integer columns are added with an implicit lower bound of 0 and upper bound of 1.
    // Non integer columns are added with an implicit lower bound of 0 and no upper bound.
    auto [insert_it, val] = columns_.emplace(column, Column{Variable{column}, integer_columns_});
    it = insert_it;
  }
  if (row == obj_row_) {
    if (!config().skip_optimise()) obj_.emplace_back(it->second.var, std::move(value));
    DELPI_TRACE_FMT("Updated obj function {}", row);
    return;
  }
  rows_.at(row).addends.emplace_back(it->second.var, std::move(value));
  DELPI_TRACE_FMT("Updated row {}", row);
}

void MpsDriver::AddRhs(const std::string &rhs, const std::string &row, mpq_class value) {
  DELPI_TRACE_FMT("Driver::AddRhs {} {} {}", rhs, row, value);
  if (!VerifyStrictRhs(rhs)) return;
  try {
    switch (Row &row_data = rows_.at(row); row_data.sense) {
      case SenseType::L:
        row_data.ub = std::move(value);
        break;
      case SenseType::G:
        row_data.lb = std::move(value);
        break;
      case SenseType::E:
        row_data.lb = row_data.ub = std::move(value);
        break;
      case SenseType::N:
        DELPI_WARN("SenseType N is used only for objective function. No action to take");
        break;
      default:
        DELPI_UNREACHABLE();
    }
  } catch (const std::out_of_range &) {
    DELPI_RUNTIME_ERROR_FMT("Row {} not found", row);
  }
  DELPI_TRACE_FMT("Updated rhs {}", row);
}

void MpsDriver::AddRange(const std::string &rhs, const std::string &row, mpq_class value) {
  DELPI_TRACE_FMT("Driver::AddRange {} {} {}", rhs, row, value);
  if (!VerifyStrictRhs(rhs)) return;
  try {
    switch (Row &row_data = rows_.at(row); row_data.sense) {
      case SenseType::L:
        mpq_abs(value.get_mpq_t(), value.get_mpq_t());
        row_data.lb = row_data.ub.value_or(0) - value;
        break;
      case SenseType::G:
        mpq_abs(value.get_mpq_t(), value.get_mpq_t());
        row_data.ub = row_data.lb.value_or(0) + value;
        break;
      case SenseType::E:
        if (value > 0) {
          *row_data.ub += value;
        } else {
          *row_data.lb += value;
        }
        break;
      case SenseType::N:
        DELPI_WARN("SenseType N is used only for objective function. No action to take");
        break;
      default:
        DELPI_UNREACHABLE();
    }
  } catch (const std::out_of_range &) {
    DELPI_RUNTIME_ERROR_FMT("Row {} not found", row);
  }
}

void MpsDriver::AddBound(const BoundType bound_type, const std::string &bound, const std::string &column,
                         mpq_class value) {
  DELPI_TRACE_FMT("Driver::AddBound {} {} {} {}", bound_type, bound, column, value);
  if (!VerifyStrictBound(bound)) return;
  try {
    switch (Column &column_data = columns_.at(column); bound_type) {
      case BoundType::UI:
        column_data.is_integer = true;
        [[fallthrough]];
      case BoundType::UP:
        column_data.ub = std::move(value);
        break;
      case BoundType::LI:
        column_data.is_integer = true;
        column_data.is_infinite_ub_integer = true;
        [[fallthrough]];
      case BoundType::LO:
        column_data.lb = std::move(value);
        break;
      case BoundType::FX:
        column_data.lb = column_data.ub = std::move(value);
        break;
      default:
        DELPI_UNREACHABLE();
    }
  } catch (const std::out_of_range &) {
    DELPI_RUNTIME_ERROR_FMT("Column {} not found", column);
  }

  DELPI_TRACE_FMT("Updated bound {}", column);
}

void MpsDriver::AddBound(const BoundType bound_type, const std::string &bound, const std::string &column) {
  DELPI_TRACE_FMT("Driver::AddBound {} {} {}", bound_type, bound, column);
  if (!VerifyStrictBound(bound)) return;
  try {
    switch (Column &column_data = columns_.at(column); bound_type) {
      case BoundType::BV:
        column_data.lb = 0;
        column_data.ub = 1;
        break;
      case BoundType::FR:
      case BoundType::MI:
        column_data.is_infinite_lb = true;
        break;
      case BoundType::PL:
        column_data.is_infinite_ub_integer = true;
        break;
      default:
        DELPI_UNREACHABLE();
    }
  } catch (const std::out_of_range &) {
    DELPI_RUNTIME_ERROR_FMT("Column {} not found", column);
  }

  DELPI_TRACE_FMT("Updated bound {}", column);
}
void MpsDriver::SetMarker([[maybe_unused]] const std::string &name, const std::string &keyword) {
  DELPI_TRACE_FMT("Driver::SetMarker({} {})", name, keyword);
  if (keyword == "INTORG") {
    DELPI_DEBUG("Integers start");
    integer_columns_ = true;
    return;
  }
  if (keyword == "INTEND") {
    DELPI_DEBUG("Integers end");
    integer_columns_ = false;
    return;
  }
  DELPI_WARN_FMT("Unknown marker '{}'. Ignoring", keyword);
}

void MpsDriver::End() {
  DELPI_DEBUG_FMT("Driver::EndData reached end of file {}", problem_name_);
  DELPI_DEBUG_FMT("Found {} variables and {} constraints", columns_.size(), rows_.size());
  static const mpq_class zero{0};
  static const mpq_class one{1};

  lp_solver_.ReserveColumns(columns_.size());
  for (const auto &[name, column_data] : columns_) {
    // The lower bound is either
    // - set explicitly
    // - negative infinity if an infinite bound has been encountered or the upper bound is negative
    // - 0 otherwise
    const mpq_class &lb = column_data.lb.has_value()                                     ? column_data.lb.value()
                          : column_data.is_infinite_lb || column_data.ub.value_or(0) < 0 ? lp_solver_.ninfinity()
                                                                                         : zero;
    // The upper bound is either
    // - set explicitly
    // - positive infinity if no explicit upper bound has been set and the variable is not an integer
    // - positive infinity if no explicit upper bound has been set and an explicit lower bound has been set via LI
    // - 1 if no explicit upper bound has been set and the variable is an integer
    const mpq_class &ub = column_data.ub.has_value()                                      ? column_data.ub.value()
                          : !column_data.is_integer || column_data.is_infinite_ub_integer ? lp_solver_.infinity()
                                                                                          : one;
    lp_solver_.AddColumn(column_data.var, lb, ub);
  }

  lp_solver_.ReserveRows(rows_.size());
  for (const auto &[row, row_data] : rows_) {
    if (row_data.addends.empty()) continue;  // No point in adding empty rows
    if (row_data.sense != SenseType::N && !row_data.lb.has_value() && !row_data.ub.has_value()) {
      DELPI_TRACE_FMT("Row {} has no RHS. Adding 0", row);
      AddRhs(rhs_name_, row, 0);
    }
    lp_solver_.AddRow(row_data.addends, row_data.lb.value_or(lp_solver_.ninfinity()),
                      row_data.ub.value_or(lp_solver_.infinity()));
  }

  if (is_min_) {
    lp_solver_.Minimise(obj_);
  } else {
    lp_solver_.Maximise(obj_);
  }
}

}  // namespace delpi::mps
