/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "QsoptexLpSolver.h"

#include <iostream>
#include <span>
#include <unordered_set>

#include "delpi/util/error.h"
#include "delpi/util/logging.h"

namespace delpi {

namespace {}  // namespace

extern "C" void QsoptexPartialSolutionCb(mpq_QSdata const* /*prob*/, const mpq_t* x, const mpq_t* const y,
                                         const mpq_t obj_lb, const mpq_t obj_up, const mpq_t diff, const mpq_t delta,
                                         void* data) {
  DELPI_DEBUG_FMT("QsoptexLpSolver::QsoptexPartialSolutionCb called with objective value in [{}, {}]",
                  mpq_class{obj_lb}, mpq_class{obj_up});
  const QsoptexLpSolver& lp_solver = *static_cast<QsoptexLpSolver*>(data);
  if (lp_solver.partial_solve_cb())
    lp_solver.partial_solve_cb()(lp_solver, LpResult::DELTA_OPTIMAL, gmp::ToMpqVector(x, lp_solver.num_columns()),
                                 gmp::ToMpqVector(y, lp_solver.num_rows()), mpq_class{obj_lb}, mpq_class{obj_up},
                                 mpq_class{diff}, mpq_class{delta});
}

QsoptexLpSolver::QsoptexLpSolver(Config config, const std::string& class_name)
    : LpSolver{0, 0, std::move(config), class_name}, qsx_{nullptr}, ray_{0}, x_{0} {
  qsopt_ex::QSXStart();
  ninfinity_ = mpq_class{mpq_NINFTY};
  infinity_ = mpq_class{mpq_INFTY};

  qsx_ = mpq_QScreate_prob(nullptr, QS_MIN);
  DELPI_ASSERT(qsx_ != nullptr, "Failed to create QSopt_ex problem");
  if (config_.verbose_simplex() > 3) {
    DELPI_RUNTIME_ERROR("With --lp-solver qsoptex, maximum value for --verbose-simplex is 3");
  }
  [[maybe_unused]] const int status = mpq_QSset_param(qsx_, QS_PARAM_SIMPLEX_DISPLAY, config_.verbose_simplex());
  DELPI_ASSERT(!status, "Invalid status");
  DELPI_DEBUG_FMT("QsoptexTheorySolver::QsoptexTheorySolver: precision = {}", config_.precision());
}

QsoptexLpSolver::~QsoptexLpSolver() {
  mpq_QSfree_prob(qsx_);
  qsopt_ex::QSXFinish();
}

int QsoptexLpSolver::num_columns() const { return mpq_QSget_colcount(qsx_); }
int QsoptexLpSolver::num_rows() const { return mpq_QSget_rowcount(qsx_); }

Column QsoptexLpSolver::column(ColumnIndex column_idx) const {
  DELPI_ASSERT(column_idx < num_columns(), "Column index out of bounds");
  qsopt_ex::MpqArray obj, lb, ub;

  [[maybe_unused]] const int status =
      mpq_QSget_columns_list(qsx_, 1, &column_idx, nullptr, nullptr, nullptr, nullptr, obj, lb, ub, nullptr);
  DELPI_ASSERT(!status, "Invalid status");

  Column column{};
  column.var = col_to_var_.at(column_idx);
  // If there is a value, move it to the optional field, otherwise free the memory
  if (!mpq_equal(lb[0], mpq_NINFTY)) column.lb = std::move(gmp::ToMpqClass(lb[0]));
  if (!mpq_equal(ub[0], mpq_INFTY)) column.ub = std::move(gmp::ToMpqClass(ub[0]));
  if (gmp::ToMpqClass(obj[0]) != 0) column.obj = std::move(gmp::ToMpqClass(obj[0]));

  return column;
}
Row QsoptexLpSolver::row(RowIndex row_idx) const {
  DELPI_ASSERT(row_idx < num_rows(), "Row index out of bounds");
  qsopt_ex::MpqArray row_val, rhs;
  int *row_cnt = nullptr, *row_ind = nullptr;
  char* sense = nullptr;

  [[maybe_unused]] const int status =
      mpq_QSget_rows_list(qsx_, 1, &row_idx, &row_cnt, nullptr, &row_ind, row_val, rhs, &sense, nullptr);
  DELPI_ASSERT(!status, "Invalid status");

  Row row{};
  const int non_zero_coefficient_count = row_cnt[0];
  for (int i = 0; i < non_zero_coefficient_count; i++) {
    row.addends.emplace_back(col_to_var_.at(row_ind[i]), std::move(gmp::ToMpqClass(row_val[i])));
    DELPI_DEBUG_FMT("QsoptexTheorySolver::row: row[{}]({}) = {} * {}", row_idx, i, row.addends.back().second,
                    row.addends.back().first);
  }

  switch (sense[0]) {
    case 'G':
      row.lb = std::move(gmp::ToMpqClass(rhs[0]));
      break;
    case 'L':
      row.ub = std::move(gmp::ToMpqClass(rhs[0]));
      break;
    case 'E':
      row.lb = gmp::ToMpqClass(rhs[0]);
      row.ub = std::move(gmp::ToMpqClass(rhs[0]));
      break;
    default:
      DELPI_UNREACHABLE();
  }

  mpq_QSfree(row_cnt);
  mpq_QSfree(row_ind);
  mpq_QSfree(sense);

  return row;
}

LpSolver::ColumnIndex QsoptexLpSolver::AddColumn(const Variable& var, const mpq_class& obj, const mpq_class& lb,
                                                 const mpq_class& ub) {
  DELPI_ASSERT(!var_to_col_.contains(var), "Variable already exists in the LP.");
  const int column_idx = num_columns();
  var_to_col_.emplace(var, column_idx);
  col_to_var_.emplace_back(var);
  [[maybe_unused]] const int status = mpq_QSnew_col(qsx_, obj.get_mpq_t(), lb.get_mpq_t(), ub.get_mpq_t(), nullptr);
  DELPI_ASSERT(!status, "Invalid status");
  return column_idx;
}
LpSolver::RowIndex QsoptexLpSolver::AddRow(const std::vector<Expression::Addend>& addends, const mpq_class& lb,
                                           const mpq_class& ub) {
  // Check if we can add a simple bound instead of a row
  if (SetSimpleBoundInsteadOfAddRow(addends, lb, ub)) return num_rows() - 1;

  // Add the row to the LP. If the row is bounded both ways with an equality, we can add it in one go.
  if (lb == ub) {
    [[maybe_unused]] const int status = mpq_QSnew_row(qsx_, lb.get_mpq_t(), 'E', nullptr);
    DELPI_ASSERT(!status, "Invalid status");
    SetRowCoeff(num_rows() - 1, addends);
    return num_rows() - 1;
  }
  // Else, add the two bounds separately. Note that this introduces 2 new rows.
  if (!mpq_equal(lb.get_mpq_t(), mpq_NINFTY)) {
    [[maybe_unused]] const int status = mpq_QSnew_row(qsx_, lb.get_mpq_t(), 'G', nullptr);
    DELPI_ASSERT(!status, "Invalid status");
    SetRowCoeff(num_rows() - 1, addends);
  }
  if (!mpq_equal(ub.get_mpq_t(), mpq_INFTY)) {
    [[maybe_unused]] const int status = mpq_QSnew_row(qsx_, ub.get_mpq_t(), 'L', nullptr);
    DELPI_ASSERT(!status, "Invalid status");
    SetRowCoeff(num_rows() - 1, addends);
  }
  return num_rows() - 1;
}

LpSolver::RowIndex QsoptexLpSolver::AddRow(const Expression::Addends& lhs, const FormulaKind sense,
                                           const mpq_class& rhs) {
  char qsoptex_sense;
  switch (sense) {
    case FormulaKind::Leq:
      qsoptex_sense = 'L';
      break;
    case FormulaKind::Eq:
      qsoptex_sense = 'E';
      break;
    case FormulaKind::Geq:
      qsoptex_sense = 'G';
      break;
    default:
      DELPI_UNREACHABLE();
  }
  [[maybe_unused]] const int status = mpq_QSnew_row(qsx_, rhs.get_mpq_t(), qsoptex_sense, nullptr);
  DELPI_ASSERT(!status, "Invalid status");
  const int row_idx = num_rows() - 1;
  SetRowCoeff(row_idx, lhs);
  return row_idx;
}
void QsoptexLpSolver::SetBound(const Variable var, const mpq_class& lb, const mpq_class& ub) {
  if (lb == ub) {
    [[maybe_unused]] const int status = mpq_QSchange_bound(qsx_, var_to_col_.at(var), 'B', lb.get_mpq_t());
    DELPI_ASSERT(!status, "Invalid status");
    return;
  }
  [[maybe_unused]] const int status1 = mpq_QSchange_bound(qsx_, var_to_col_.at(var), 'L', lb.get_mpq_t());
  DELPI_ASSERT(!status1, "Invalid status");
  [[maybe_unused]] const int status2 = mpq_QSchange_bound(qsx_, var_to_col_.at(var), 'U', ub.get_mpq_t());
  DELPI_ASSERT(!status2, "Invalid status");
}

void QsoptexLpSolver::SetObjective(int column, const mpq_class& value) {
  DELPI_ASSERT_FMT(column < num_columns(), "Column index out of bounds: {} >= {}", column, num_columns());
  [[maybe_unused]] const int status = mpq_QSchange_objcoef(qsx_, column, mpq_class{value}.get_mpq_t());
  DELPI_ASSERT(!status, "Invalid status");
}

LpResult QsoptexLpSolver::SolveCore(mpq_class& precision, const bool store_solution) {
  // x: must be allocated/deallocated using QSopt_ex.
  // Should have room for the (rowcount) "logical" variables, which come after the (colcount) "structural" variables.
  x_.Resize(num_columns());
  ray_.Resize(num_rows());

  int lp_status = -1;
  const int status = QSdelta_full_solver(qsx_, precision.get_mpq_t(), x_, ray_, obj_lb_.get_mpq_t(),
                                         obj_ub_.get_mpq_t(), nullptr, PRIMAL_SIMPLEX, &lp_status,
                                         config_.continuous_output() ? QsoptexPartialSolutionCb : nullptr, this);
  if (status) {
    DELPI_RUNTIME_ERROR_FMT("QSopt_ex returned {}", status);
    return LpResult::ERROR;
  }

  DELPI_DEBUG_FMT("DeltaQsoptexTheorySolver::CheckSat: QSopt_ex has returned with precision = {}", precision);

  switch (lp_status) {
    case QS_LP_OPTIMAL:
    case QS_LP_DELTA_OPTIMAL:
      if (store_solution) UpdateFeasible();
      return lp_status == QS_LP_OPTIMAL ? LpResult::OPTIMAL : LpResult::DELTA_OPTIMAL;
    case QS_LP_UNBOUNDED:
      if (store_solution) UpdateFeasible();
      return LpResult::UNBOUNDED;
    case QS_LP_INFEASIBLE:
#if 0
      if (store_solution) UpdateInfeasible();
#endif
      return LpResult::INFEASIBLE;
    case QS_LP_UNSOLVED:
      DELPI_WARN("DeltaQsoptexTheorySolver::CheckSat: QSopt_ex failed to return a result");
      return LpResult::ERROR;
    default:
      DELPI_UNREACHABLE();
  }
}

void QsoptexLpSolver::UpdateFeasible() {
  DELPI_ASSERT(solution_.empty(), "Solution must be empty");
  DELPI_ASSERT(dual_solution_.empty(), "Dual solution must be empty");
  // Set the feasible information
  const int colcount = num_columns();
  const int rowcount = num_rows();
  solution_.reserve(colcount);
  dual_solution_.reserve(rowcount);

  for (int i = 0; i < colcount; i++) solution_.emplace_back(x_[i]);
  for (int i = 0; i < rowcount; i++) dual_solution_.emplace_back(ray_[i]);
}
#if 0
void QsoptexLpSolver::UpdateInfeasible() {
  DELPI_ASSERT(infeasible_rows_.empty(), "Infeasible rows must be empty");
  DELPI_ASSERT(infeasible_bounds_.empty(), "Infeasible bounds must be empty");
  // Set the infeasible information
  const int rowcount = num_rows();
  const int colcount = num_columns();

  // Add the non-zero rows to the infeasible core
  for (int i = 0; i < rowcount; i++) {
    if (mpq_sgn(ray_[i]) == 0) continue;
    DELPI_TRACE_FMT("QsoptexLpSolver::NotifyInfeasible: ray[{}] = {}", i, gmp::ToMpqClass(ray_[i]));
    infeasible_rows_.emplace_back(i);
  }
  //  Multiply the Farkas ray by the row coefficients to get the column violations: ray * A
  //  If the result is non-zero, the sign indicates the bound that caused the violation.
  mpq_class col_violation{0};
  mpq_t row_coeff;
  mpq_init(row_coeff);
  for (int i = 0; i < colcount; i++) {
    col_violation = 0;
    for (int j = 0; j < rowcount; j++) {
      mpq_QSget_coef(qsx_, j, i, &row_coeff);
      col_violation += gmp::ToMpqClass(ray_[j]) * gmp::ToMpqClass(row_coeff);
    }
    if (col_violation == 0) continue;
    DELPI_TRACE_FMT("QsoptexLpSolver::NotifyInfeasible: {}[{}] = {}", col_to_var_.at(i), i, col_violation);
    infeasible_bounds_.emplace_back(i, col_violation > 0);
  }
  mpq_clear(row_coeff);
}
#endif

template <TypedIterable<std::pair<const Variable, mpq_class>> T>
void QsoptexLpSolver::SetRowCoeff(int row, const T& literal_monomials) {
  for (const auto& [var, coeff] : literal_monomials) SetVarCoeff(row, var, coeff);
}

void QsoptexLpSolver::SetVarCoeff(const int row, const Variable& var, const mpq_class& value) const {
  DELPI_ASSERT_FMT(var_to_col_.contains(var), "Variable {} not found in the LP. Did you add it before?", var);
  const int column = var_to_col_.at(var);
  // Variable has the coefficients too large
  if (value <= ninfinity_ || value >= infinity_) DELPI_RUNTIME_ERROR_FMT("LP coefficient too large: {}", value);

  mpq_t c_value;
  mpq_init(c_value);
  mpq_set(c_value, value.get_mpq_t());
  [[maybe_unused]] const int status = mpq_QSchange_coef(qsx_, row, column, c_value);
  DELPI_ASSERT(!status, "Invalid status");
  mpq_clear(c_value);
}

#ifndef NDEBUG
void QsoptexLpSolver::Dump() {
  mpq_QSdump_prob(qsx_);
  mpq_QSdump_basis(qsx_);
  mpq_QSdump_bfeas(qsx_);
}
#endif

void QsoptexLpSolver::SetCoefficient(const int row, const int column, const mpq_class& value) {
  DELPI_ASSERT_FMT(row < num_rows(), "Row index out of bounds: {} >= {}", row, num_rows());
  DELPI_ASSERT_FMT(column < num_columns(), "Column index out of bounds: {} >= {}", column, num_columns());
  DELPI_ASSERT_FMT(value <= infinity_ && value >= ninfinity_, "LP coefficient too large: {}", value);

  [[maybe_unused]] const int status = mpq_QSchange_coef(qsx_, row, column, mpq_class{value}.get_mpq_t());
  DELPI_ASSERT(!status, "Invalid status");
}

template void QsoptexLpSolver::SetRowCoeff(int, const std::vector<std::pair<const Variable, mpq_class>>&);
template void QsoptexLpSolver::SetRowCoeff(int, const std::set<std::pair<const Variable, mpq_class>>&);
template void QsoptexLpSolver::SetRowCoeff(int, const std::unordered_set<std::pair<const Variable, mpq_class>>&);
template void QsoptexLpSolver::SetRowCoeff(int, const std::span<std::pair<const Variable, mpq_class>>&);
template void QsoptexLpSolver::SetRowCoeff(int, const std::map<Variable, mpq_class>&);
template void QsoptexLpSolver::SetRowCoeff(int, const std::unordered_map<Variable, mpq_class>&);

}  // namespace delpi