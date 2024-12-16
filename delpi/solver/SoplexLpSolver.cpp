/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/solver/SoplexLpSolver.h"

#include <map>
#include <set>
#include <span>  // NOLINT(build/include_order): c++20 header
#include <unordered_map>
#include <unordered_set>

#include "delpi/util/error.h"
#include "delpi/util/logging.h"

namespace delpi {

using SoplexStatus = soplex::SPxSolver::Status;

SoplexLpSolver::SoplexLpSolver(Config config, const std::string& class_name)
    : LpSolver{-soplex::infinity, soplex::infinity, std::move(config), class_name},
      consolidated_{false},
      spx_{},
      rninfinity_{-soplex::infinity},
      rinfinity_{soplex::infinity} {
  // Default SoPlex parameters
  spx_.setRealParam(soplex::SoPlex::FEASTOL, config_.precision());
  spx_.setBoolParam(soplex::SoPlex::RATREC, false);
  spx_.setIntParam(soplex::SoPlex::READMODE, soplex::SoPlex::READMODE_RATIONAL);
  spx_.setIntParam(soplex::SoPlex::SOLVEMODE, soplex::SoPlex::SOLVEMODE_RATIONAL);
  spx_.setIntParam(soplex::SoPlex::CHECKMODE, soplex::SoPlex::CHECKMODE_RATIONAL);
  spx_.setIntParam(soplex::SoPlex::SYNCMODE, soplex::SoPlex::SYNCMODE_AUTO);
  spx_.setIntParam(soplex::SoPlex::SIMPLIFIER, soplex::SoPlex::SIMPLIFIER_INTERNAL);
  spx_.setIntParam(soplex::SoPlex::VERBOSITY, config_.verbose_simplex());
  // Default is maximise.
  spx_.setIntParam(soplex::SoPlex::OBJSENSE, soplex::SoPlex::OBJSENSE_MINIMIZE);
  // Enable precision boosting
  bool enable_precision_boosting = config_.lp_mode() != Config::LpMode::PURE_ITERATIVE_REFINEMENT;
  spx_.setBoolParam(soplex::SoPlex::ADAPT_TOLS_TO_MULTIPRECISION, enable_precision_boosting);
  spx_.setBoolParam(soplex::SoPlex::PRECISION_BOOSTING, enable_precision_boosting);
  spx_.setIntParam(soplex::SoPlex::RATFAC_MINSTALLS, enable_precision_boosting ? 0 : 2);
  // Enable iterative refinement
  bool enable_iterative_refinement = config_.lp_mode() != Config::LpMode::PURE_PRECISION_BOOSTING;
  spx_.setBoolParam(soplex::SoPlex::ITERATIVE_REFINEMENT, enable_iterative_refinement);
  DELPI_DEBUG_FMT(
      "SoplexTheorySolver::SoplexTheorySolver: precision = {}, precision_boosting = {}, iterative_refinement = {}",
      config_.precision(), enable_precision_boosting, enable_iterative_refinement);
}

int SoplexLpSolver::num_columns() const { return consolidated_ ? spx_.numColsRational() : spx_cols_.num(); }
int SoplexLpSolver::num_rows() const { return consolidated_ ? spx_.numRowsRational() : spx_rows_.num(); }

Column SoplexLpSolver::column(const ColumnIndex column_idx) const {
  DELPI_ASSERT(column_idx < num_columns(), "Column index out of bounds");
  const soplex::Rational& lower = consolidated_ ? spx_.lowerRational(column_idx) : spx_cols_.lower(column_idx);
  const soplex::Rational& upper = consolidated_ ? spx_.upperRational(column_idx) : spx_cols_.upper(column_idx);
  const soplex::Rational& obj = consolidated_ ? spx_.objRational(column_idx) : spx_cols_.maxObj(column_idx);
  Column column{};
  column.var = col_to_var_.at(column_idx);
  if (lower > -soplex::infinity) column.lb = std::move(gmp::ToMpqClass(lower.backend().data()));
  if (upper < soplex::infinity) column.ub = std::move(gmp::ToMpqClass(upper.backend().data()));
  if (!obj.is_zero()) column.obj = std::move(gmp::ToMpqClass(obj.backend().data()));

  return column;
}
Row SoplexLpSolver::row(const RowIndex row_idx) const {
  DELPI_ASSERT(row_idx < num_rows(), "Row index out of bounds");
  const soplex::Rational lhs = consolidated_ ? spx_.lhsRational(row_idx) : spx_rows_.lhs(row_idx);
  const soplex::Rational rhs = consolidated_ ? spx_.rhsRational(row_idx) : spx_rows_.rhs(row_idx);
  Row row{};
  if (lhs > -soplex::infinity) row.lb = std::move(gmp::ToMpqClass(lhs.backend().data()));
  if (rhs < soplex::infinity) row.ub = std::move(gmp::ToMpqClass(rhs.backend().data()));

  const soplex::SVectorRational addends =
      consolidated_ ? spx_.rowVectorRational(row_idx) : spx_rows_.rowVector(row_idx);
  for (int i = 0; i < addends.size(); ++i) {
    const soplex::Rational& coeff = addends.value(i);
    const Variable& var = col_to_var_.at(addends.index(i));
    row.addends.emplace_back(var, std::move(gmp::ToMpqClass(coeff.backend().data())));
  }
  return row;
}

void SoplexLpSolver::ReserveColumns(const int num_columns) {
  LpSolver::ReserveColumns(num_columns);
  spx_cols_ = soplex::LPColSetRational(num_columns, num_columns);
}
void SoplexLpSolver::ReserveRows(const int num_rows) {
  LpSolver::ReserveRows(num_rows);
  spx_rows_ = soplex::LPRowSetRational(num_rows, num_rows);
}

LpSolver::ColumnIndex SoplexLpSolver::AddColumn(const Variable& var, const mpq_class& obj, const mpq_class& lb,
                                                const mpq_class& ub) {
  DELPI_ASSERT_FMT(!var_to_col_.contains(var), "Variable '{}' already exists in the LP.", var);
  const ColumnIndex column_idx = num_columns();
  var_to_col_.emplace(var, column_idx);
  col_to_var_.emplace_back(var);
  const soplex::LPColRational col_rational(obj.get_mpq_t(), soplex::DSVectorRational(), ub.get_mpq_t(), lb.get_mpq_t());
  // Add the column to the LP
  if (consolidated_)
    spx_.addColRational(col_rational);
  else
    spx_cols_.add(col_rational);
  return column_idx;
}
LpSolver::RowIndex SoplexLpSolver::AddRow(const std::vector<Expression::Addend>& addends, const mpq_class& lb,
                                          const mpq_class& ub) {
  // Check if we can add a simple bound instead of a row
  if (SetSimpleBoundInsteadOfAddRow(addends, lb, ub)) return num_rows() - 1;

  const soplex::LPRowRational row_rational(lb.get_mpq_t(), ParseRowCoeff(addends), ub.get_mpq_t());
  if (consolidated_)
    spx_.addRowRational(row_rational);
  else
    spx_rows_.add(row_rational);
  return num_rows() - 1;
}

LpSolver::RowIndex SoplexLpSolver::AddRow(const Expression::Addends& lhs, const FormulaKind sense,
                                          const mpq_class& rhs) {
  DELPI_ASSERT(sense == FormulaKind::Leq || sense == FormulaKind::Eq || sense == FormulaKind::Geq, "Invalid row sense");
  const soplex::LPRowRational row_rational((sense == FormulaKind::Leq ? ninfinity_.get_mpq_t() : rhs.get_mpq_t()),
                                           ParseRowCoeff(lhs),
                                           (sense == FormulaKind::Geq ? infinity_.get_mpq_t() : rhs.get_mpq_t()));
  if (consolidated_)
    spx_.addRowRational(row_rational);
  else
    spx_rows_.add(row_rational);
  return num_rows() - 1;
}
void SoplexLpSolver::SetBound(const Variable var, const mpq_class& lb, const mpq_class& ub) {
  if (consolidated_) {
    spx_.changeBoundsRational(var_to_col_.at(var), lb.get_mpq_t(), ub.get_mpq_t());
  } else {
    spx_cols_.lower_w()[var_to_col_.at(var)] = lb.get_mpq_t();
    spx_cols_.upper_w()[var_to_col_.at(var)] = ub.get_mpq_t();
  }
}
void SoplexLpSolver::SetCoefficient(const int row, const int column, const mpq_class& value) {
  DELPI_ASSERT(row < num_rows(), "Row index out of bounds");
  DELPI_ASSERT(column < num_columns(), "Column index out of bounds");
  DELPI_ASSERT(ninfinity_ <= value && value <= infinity_, "LP coefficient value too large");

  if (consolidated_) {
    spx_.changeElementRational(row, column, value.get_mpq_t());
  } else {
    spx_rows_.rowVector_w(row).value(column) = value.get_mpq_t();
  }

  if (DELPI_TRACE_ENABLED) {
    if (consolidated_)
      DELPI_TRACE_FMT("SoplexLpSolver::SetCoefficient: row {}: {}", row, spx_.rowVectorRational(row));
    else
      DELPI_TRACE_FMT("SoplexLpSolver::SetCoefficient: row {}: {}", row, spx_rows_.rowVector(row));
  }
}
void SoplexLpSolver::SetObjective(const int column, const mpq_class& value) {
  DELPI_ASSERT(column < num_columns(), "Column index out of bounds");
  if (consolidated_)
    spx_.changeObjRational(column, value.get_mpq_t());
  else
    spx_cols_.maxObj_w(column) = value.get_mpq_t();
}

LpResult SoplexLpSolver::SolveCore(mpq_class& precision, const bool store_solution) {
  if (!consolidated_) {
    spx_.addColsRational(spx_cols_);
    spx_.addRowsRational(spx_rows_);
    consolidated_ = true;
    spx_cols_.clear();
    spx_rows_.clear();
  }
  const SoplexStatus status = spx_.optimize();
  soplex::Rational max_violation, sum_violation;

  // The status must be OPTIMAL, UNBOUNDED, or INFEASIBLE. Anything else is an error
  if (status != SoplexStatus::OPTIMAL && status != SoplexStatus::UNBOUNDED && status != SoplexStatus::INFEASIBLE) {
    DELPI_ERROR_FMT("SoplexLpSolver::Optimise: Unexpected SoPlex return -> {}", status);
    return LpResult::ERROR;
  } else if (spx_.getRowViolationRational(max_violation, sum_violation)) {
    precision = gmp::ToMpqClass(max_violation.backend().data());
    DELPI_DEBUG_FMT("SoplexLpSolver::Optimise: SoPlex returned {}, precision = {}", status, precision);
  } else {
    DELPI_DEBUG_FMT("SoplexLpSolver::Optimise: SoPlex has returned {}", status);
  }

  switch (status) {
    case SoplexStatus::OPTIMAL:
      if (store_solution) UpdateFeasible();
      return max_violation.is_zero() ? LpResult::OPTIMAL : LpResult::DELTA_OPTIMAL;
    case SoplexStatus::UNBOUNDED:
      if (store_solution) UpdateFeasible();
      return LpResult::UNBOUNDED;
    case SoplexStatus::INFEASIBLE:
      // if (store_solution) UpdateInFeasible();
      return LpResult::INFEASIBLE;
    default:
      DELPI_UNREACHABLE();
  }
}

void SoplexLpSolver::UpdateFeasible() {
  DELPI_ASSERT(solution_.empty(), "solution_ must be empty");
  DELPI_ASSERT(dual_solution_.empty(), "dual_solution_ must be empty");
  // Set the feasible information
  const int colcount = num_columns();
  const int rowcount = num_rows();
  solution_.reserve(colcount);
  dual_solution_.reserve(rowcount);

  soplex::VectorRational solution{colcount};
  [[maybe_unused]] const bool has_sol = spx_.getPrimalRational(solution);
  DELPI_ASSERT(has_sol, "has_sol must be true");
  DELPI_ASSERT(solution.dim() >= colcount, "x.dim() must be >= colcount");
  for (int i = 0; i < solution.dim(); i++) solution_.emplace_back(gmp::ToMpqClass(solution[i].backend().data()));

  soplex::VectorRational dual{rowcount};
  [[maybe_unused]] const bool has_dual = spx_.getDualRational(dual);
  DELPI_ASSERT(has_dual, "has_dual must be true");
  for (int i = 0; i < rowcount; i++) dual_solution_.emplace_back(gmp::ToMpqClass(dual[i].backend().data()));

  obj_lb_ = obj_ub_ = gmp::ToMpqClass(spx_.objValueRational().backend().data());
}

#if 0
void SoplexLpSolver::UpdateInfeasible() {
  DELPI_ASSERT(infeasible_rows_.empty(), "infeasible_rows_ must be empty");
  DELPI_ASSERT(infeasible_bounds_.empty(), "infeasible_bounds_ must be empty");
  // Set the infeasible information
  const int rowcount = num_rows();
  const int colcount = num_columns();

  soplex::VectorRational farkas_ray{rowcount};
  DELPI_ASSERT(farkas_ray.dim() == num_rows(), "farkas_ray must have the same dimension as the rows");
  // Get the Farkas ray to identify which rows are responsible for the conflict
  [[maybe_unused]] bool res = spx_.getDualFarkasRational(farkas_ray);
  DELPI_ASSERT(res, "getDualFarkasRational() must return true");

  // Add the non-zero rows to the infeasible core
  for (int i = 0; i < rowcount; i++) {
    if (farkas_ray[i].is_zero()) continue;
    DELPI_TRACE_FMT("SoplexLpSolver::NotifyInfeasible: ray[{}] = {}", i, farkas_ray[i]);
    infeasible_rows_.emplace_back(i);
  }
  //  Multiply the Farkas ray by the row coefficients to get the column violations: ray * A
  //  If the result is non-zero, the sign indicates the bound that caused the violation.
  soplex::Rational col_violation{0};
  for (int i = 0; i < colcount; i++) {
    col_violation = 0;
    for (int j = 0; j < rowcount; j++) {
      col_violation += farkas_ray[j] * spx_.rowVectorRational(j)[i];
    }
    if (col_violation.is_zero()) continue;
    if (DELPI_TRACE_ENABLED && static_cast<std::size_t>(i) < col_to_var_.size())
      DELPI_TRACE_FMT("SoplexLpSolver::NotifyInfeasible: {}[{}] = {}", col_to_var_.at(i), i, col_violation);
    infeasible_bounds_.emplace_back(i, col_violation < 0);
  }
}
#endif

template <TypedIterable<std::pair<const Variable, mpq_class>> T>
soplex::DSVectorRational SoplexLpSolver::ParseRowCoeff(const T& literal_monomials) {
  soplex::DSVectorRational coeffs;
  for (const auto& [var, coeff] : literal_monomials) SetVarCoeff(coeffs, var, coeff);
  return coeffs;
}

void SoplexLpSolver::SetVarCoeff(soplex::DSVectorRational& coeffs, const Variable& var, const mpq_class& value) const {
  const auto it = var_to_col_.find(var);
  if (it == var_to_col_.end()) DELPI_RUNTIME_ERROR_FMT("Undefined variable in the SoPlex LP solver: {}", var);
  if (value <= ninfinity_ || value >= infinity_) {
    DELPI_RUNTIME_ERROR_FMT("LP coefficient too large for SoPlex: {} <= {} <= {}", ninfinity_, value, infinity_);
  }
  coeffs.add(it->second, gmp::ToMpq(value));
}

#ifndef NDEBUG
void SoplexLpSolver::Dump() { spx_.writeFileRational("~/delpi.temp.dump.soplex.lp"); }
#endif

template soplex::DSVectorRational SoplexLpSolver::ParseRowCoeff(
    const std::vector<std::pair<const Variable, mpq_class>>& literal_monomials);
template soplex::DSVectorRational SoplexLpSolver::ParseRowCoeff(
    const std::set<std::pair<const Variable, mpq_class>>& literal_monomials);
template soplex::DSVectorRational SoplexLpSolver::ParseRowCoeff(
    const std::unordered_set<std::pair<const Variable, mpq_class>>& literal_monomials);
template soplex::DSVectorRational SoplexLpSolver::ParseRowCoeff(
    const std::span<std::pair<const Variable, mpq_class>>& literal_monomials);
template soplex::DSVectorRational SoplexLpSolver::ParseRowCoeff(const std::map<Variable, mpq_class>& literal_monomials);
template soplex::DSVectorRational SoplexLpSolver::ParseRowCoeff(
    const std::unordered_map<Variable, mpq_class>& literal_monomials);

}  // namespace delpi
