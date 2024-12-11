/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */

#include "Driver.h"

#include <iostream>

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

void MpsDriver::AddRow(Sense sense, const std::string &row) {
  DELPI_TRACE_FMT("Driver::AddRow {} {}", sense, row);
  if (sense == Sense::N && obj_row_.empty()) {
    DELPI_DEBUG("Objective row not found. Adding the first row with sense N as objective row");
    obj_row_ = row;
  }
  rows_[row];
}

void MpsDriver::AddColumn(const std::string &column, const std::string &row, mpq_class value) {
  DELPI_TRACE_FMT("Driver::AddColumn {} {} {}", row, column, value);
  auto it = columns_.find(column);
  if (columns_.end() == it) {
    DELPI_TRACE_FMT("Added column {}", column);
    auto [insert_it, val] =
        columns_.emplace(column, Column{Variable{column}, std::nullopt, std::nullopt, std::nullopt});
    it = insert_it;
  }
  if (!config().optimize() && row == obj_row_) return;
  rows_.at(row).addends.emplace_back(it->second.var, std::move(value));
  DELPI_TRACE_FMT("Updated row {}", row);
}

void MpsDriver::AddRhs(const std::string &rhs, const std::string &row, mpq_class value) {
  DELPI_TRACE_FMT("Driver::AddRhs {} {} {}", rhs, row, value);
  if (!VerifyStrictRhs(rhs)) return;
  rhs_values_[row] = value;
  Expression row_expression = ExpressionAddFactory{0, rows_[row]}.GetExpression();
  try {
    switch (row_senses_.at(row)) {
      case Sense::L:
        rhs_[row] = row_expression <= value;
        break;
      case Sense::G:
        rhs_[row] = row_expression >= value;
        break;
      case Sense::E:
        rhs_[row] = row_expression == value;
        break;
      case Sense::N:
        DELPI_WARN("Sense N is used only for objective function. No action to take");
        break;
      default:
        DELPI_UNREACHABLE();
    }
  } catch (const std::out_of_range &) {
    DELPI_RUNTIME_ERROR_FMT("Row {} not found", row);
  }
  DELPI_TRACE_FMT("Updated rhs {}", rhs_[row]);
}

void MpsDriver::AddRange(const std::string &rhs, const std::string &row, mpq_class value) {
  DELPI_TRACE_FMT("Driver::AddRange {} {} {}", rhs, row, value);
  if (!VerifyStrictRhs(rhs)) return;
  try {
    Expression row_expression = ExpressionAddFactory{0, rows_[row]}.GetExpression();
    switch (row_senses_.at(row)) {
      case Sense::L:
        mpq_abs(value.get_mpq_t(), value.get_mpq_t());
        rhs_[row] &= row_expression >= mpq_class{rhs_values_[row] - value};
        break;
      case Sense::G:
        mpq_abs(value.get_mpq_t(), value.get_mpq_t());
        rhs_[row] &= row_expression <= mpq_class{rhs_values_[row] + value};
        break;
      case Sense::E:
        rhs_[row] = value > 0
                        ? row_expression >= rhs_values_[row] && row_expression <= mpq_class(rhs_values_[row] + value)
                        : row_expression >= mpq_class(rhs_values_[row] + value) && row_expression <= rhs_values_[row];
        break;
      case Sense::N:
        DELPI_WARN("Sense N is used only for objective function. No action to take");
        break;
      default:
        DELPI_UNREACHABLE();
    }
  } catch (const std::out_of_range &) {
    DELPI_RUNTIME_ERROR_FMT("Row {} not found", row);
  }
}

void MpsDriver::AddBound(BoundType bound_type, const std::string &bound, const std::string &column, mpq_class value) {
  DELPI_TRACE_FMT("Driver::AddBound {} {} {} {}", bound_type, bound, column, value);
  if (!VerifyStrictBound(bound)) return;
  try {
    switch (bound_type) {
      case BoundType::UP:
      case BoundType::UI:
        bounds_[column] &= columns_.at(column) <= value;
        if (value <= 0) skip_lower_bound_[column] = true;
        break;
      case BoundType::LO:
      case BoundType::LI:
        bounds_[column] &= columns_.at(column) >= value;
        skip_lower_bound_[column] = true;
        break;
      case BoundType::FX:
        bounds_[column] &= (columns_.at(column) >= value) && (columns_.at(column) <= value);
        skip_lower_bound_[column] = true;
        break;
      default:
        DELPI_UNREACHABLE();
    }
  } catch (const std::out_of_range &) {
    DELPI_RUNTIME_ERROR_FMT("Column {} not found", column);
  }

  DELPI_TRACE_FMT("Updated bound {}", bounds_[column]);
}

void MpsDriver::AddBound(BoundType bound_type, const std::string &bound, const std::string &column) {
  DELPI_TRACE_FMT("Driver::AddBound {} {} {}", bound_type, bound, column);
  if (!VerifyStrictBound(bound)) return;
  try {
    switch (bound_type) {
      case BoundType::BV:
        bounds_[column] = (columns_.at(column) >= 0) && (columns_.at(column) <= 1);
        skip_lower_bound_[column] = true;
        break;
      case BoundType::FR:
      case BoundType::MI:
        skip_lower_bound_[column] = true;
        break;
      case BoundType::PL:
        DELPI_DEBUG("Infinity bound, no action to take");
        break;
      default:
        DELPI_UNREACHABLE();
    }
  } catch (const std::out_of_range &) {
    DELPI_RUNTIME_ERROR_FMT("Column {} not found", column);
  }

  DELPI_TRACE_FMT("Updated bound {}", bounds_[column]);
}

void MpsDriver::End() {
  DELPI_DEBUG_FMT("Driver::EndData reached end of file {}", problem_name_);
  for (const auto &[row, sense] : row_senses_) {
    if (sense != Sense::N && rhs_.find(row) == rhs_.end()) {
      DELPI_TRACE_FMT("Row {} has no RHS. Adding 0", row);
      AddRhs(rhs_name_, row, 0);
    }
  }
  for (const auto &[column, var] : columns_) {
    if (skip_lower_bound_.find(column) == skip_lower_bound_.end()) {
      DELPI_TRACE_FMT("Column has no lower bound. Adding 0 <= {}", column);
      AddBound(BoundType::LO, bound_name_, column, 0);
    }
  }
  DELPI_DEBUG_FMT("Found {} assertions", n_assertions());
  for (const auto &[name, bound] : bounds_) {
    if (is_conjunction(bound)) {
      for (const Formula &sub_bound : get_operands(bound)) context_.Assert(sub_bound);
    } else {
      context_.Assert(bound);
    }
  }
  for (const auto &[name, row] : rhs_) {
    if (row.EqualTo(Formula::True())) continue;
    if (is_conjunction(row)) {
      for (const Formula &sub_row : get_operands(row)) context_.Assert(sub_row);
    } else {
      context_.Assert(row);
    }
  }
  if (context_.config().optimize() && !obj_row_.empty()) {
    Expression obj_expression = ExpressionAddFactory{0, rows_.at(obj_row_)}.GetExpression();
    if (is_min_) {
      context_.Minimise(obj_expression);
    } else {
      context_.Maximise(obj_expression);
    }
  }
}

}  // namespace delpi::mps
