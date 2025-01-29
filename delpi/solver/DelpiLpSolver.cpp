/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/solver/DelpiLpSolver.h"

#include <iostream>
#include <ostream>
#include <unordered_set>

#include "delpi/util/error.h"
#include "internal/Basis.h"
#include "internal/BgLinearSystemSolver.h"

namespace delpi {

namespace {
inline bool IsInfinity(const mpq_class& value) { return mpz_sgn(value.get_den_mpz_t()) == 0; }
}  // namespace

DelpiLpSolver::DelpiLpSolver(Config config, const std::string& class_name)
    : LpSolver{mpq_class{mpz_class{0}, 0}, mpq_class{mpz_class{0}, 0}, std::move(config), class_name} {}

int DelpiLpSolver::num_columns() const { return A_.cols(); }
int DelpiLpSolver::num_rows() const { return A_.rows(); }

Column DelpiLpSolver::column(const ColumnIndex column_idx) const {
  DELPI_ASSERT(column_idx < num_columns(), "Column index out of bounds");
  return {col_to_var_.at(column_idx), 0, std::nullopt,
          c_(column_idx) == 0 ? std::optional<mpq_class>{} : std::optional{c_(column_idx)}};
}
Row DelpiLpSolver::row(const RowIndex row_idx) const {
  DELPI_ASSERT(row_idx < num_rows(), "Row index out of bounds");
  const ColumnIndex columns = num_columns();
  std::vector<std::pair<Variable, mpq_class>> addends;
  addends.reserve(num_columns());
  for (int i = 0; i < columns; ++i) {
    if (A_(row_idx, i) != 0) addends.emplace_back(col_to_var_.at(i), A_(row_idx, i));
  }
  return {addends,
          row_senses_[row_idx] == FormulaKind::Eq || row_senses_[row_idx] == FormulaKind::Geq ||
                  row_senses_[row_idx] == FormulaKind::Gt
              ? std::optional<mpq_class>{b_(row_idx)}
              : std::nullopt,
          row_senses_[row_idx] == FormulaKind::Eq || row_senses_[row_idx] == FormulaKind::Leq ||
                  row_senses_[row_idx] == FormulaKind::Lt
              ? std::optional<mpq_class>{b_(row_idx)}
              : std::nullopt};
}

void DelpiLpSolver::ReserveColumns(const int num_columns) {
  LpSolver::ReserveColumns(num_columns);
  A_.conservativeResize(Eigen::NoChange, num_columns);
  // TODO(tend): Use conservativeResize to improve performance
  // c_.conservativeResize(num_columns);
}
void DelpiLpSolver::ReserveRows(const int num_rows) {
  LpSolver::ReserveRows(num_rows);
  // TODO(tend): Use conservativeResize to improve performance
  // A_.conservativeResize(num_rows, Eigen::NoChange);
}
LpSolver::ColumnIndex DelpiLpSolver::AddColumn(const Variable& var, const mpq_class& obj, const mpq_class&,
                                               const mpq_class&) {
  DELPI_ASSERT_FMT(!var_to_col_.contains(var), "Variable '{}' already exists in the LP.", var);
  DELPI_ASSERT(A_.cols() == c_.size(), "Inconsistent number of columns and objective coefficients");
  const ColumnIndex column_idx = num_columns();
  var_to_col_.emplace(var, column_idx);
  col_to_var_.emplace_back(var);
  // TODO(tend): Consider upper and lower bounds
  A_.conservativeResize(Eigen::NoChange, column_idx + 1);
  A_.rightCols(1).setZero();
  c_.conservativeResize(column_idx + 1);
  c_.tail(1).setConstant(obj);
  return column_idx;
}
LpSolver::RowIndex DelpiLpSolver::AddRow(const std::vector<Expression::Addend>& addends, const mpq_class& lb,
                                         const mpq_class& ub) {
  if (IsInfinity(lb) && IsInfinity(ub)) {
    DELPI_WARN_FMT("Ignoring unbounded row with addends: {}", addends);
    return -1;
  }
  Expression::Addends map_addends;
  for (const auto& [var, coeff] : addends) map_addends.emplace(var, coeff);
  if (lb == ub) return AddRow(map_addends, FormulaKind::Eq, lb);
  if (!IsInfinity(lb)) AddRow(map_addends, FormulaKind::Geq, lb);
  if (!IsInfinity(ub)) AddRow(map_addends, FormulaKind::Leq, ub);
  return num_rows() - 1;
}
LpSolver::RowIndex DelpiLpSolver::AddRow(const Expression::Addends& lhs, const FormulaKind sense,
                                         const mpq_class& rhs) {
  DELPI_ASSERT(sense == FormulaKind::Eq || sense == FormulaKind::Leq || sense == FormulaKind::Geq,
               "Only equality, less than or equal, and greater than or equal constraints are supported");

  const RowIndex row_idx = num_rows();
  // Set the coefficients in the A matrix
  A_.conservativeResize(row_idx + 1, Eigen::NoChange);
  A_.bottomRows(1).setZero();
  for (const auto& [var, coeff] : lhs) A_(row_idx, var_to_col_.at(var)) = coeff;
  // Set the right-hand side in the b vector
  b_.conservativeResize(row_idx + 1);
  b_.tail(1).setConstant(rhs);

  row_senses_.emplace_back(sense);
  return row_idx;
}
void DelpiLpSolver::SetBound(Variable, const mpq_class&, const mpq_class&) {
  // TODO(tend): Consider upper and lower bounds
}
void DelpiLpSolver::SetCoefficient(const RowIndex row, const ColumnIndex column, const mpq_class& value) {
  A_(row, column) = value;
}
void DelpiLpSolver::SetObjective(const int column, const mpq_class& value) { c_(column) = value; }

#ifndef NDEBUG
void DelpiLpSolver::Dump() {
  std::cout << "DelpiLpSolver{ A:\n" << A_ << ",\nb:\n" << b_ << ",\nc:\n" << c_ << "\n}" << std::endl;
}
#endif
LpResult DelpiLpSolver::SolveCore(mpq_class&, bool) {
  DELPI_ASSERT(static_cast<std::size_t>(A_.rows()) == row_senses_.size(), "Inconsistent number of rows and senses");

  auto [slack_A, slack_c] = ToSlackForm();
  internal::Basis<mpq_class> basis(slack_A);
  DELPI_DEV_FMT("slack_A:\n{}\nslack_c:\n{}", slack_A, slack_c);

  const LpResult feasibility_check = FeasibilityCheck(slack_A, basis);
  DELPI_DEV_FMT("Feasibility check: {}. Feasible basis:\n{}\nFesible sol: {}", feasibility_check, basis, x_);
  if (feasibility_check == LpResult::INFEASIBLE) return feasibility_check;

  const LpResult result = InternalSolve(slack_A, b_, slack_c, basis);

  // Drop the slack variables from the solution
  x_ = x_.tail(num_columns()).eval();
  DELPI_DEV_FMT("Result check: {}. Basis:\n{}\nSolution: {}, c: {}", result, basis, x_, c_);
  if (result == LpResult::OPTIMAL) obj_lb_ = obj_ub_ = c_.transpose() * x_;
  return result;
}

LpResult DelpiLpSolver::InternalSolve(const Matrix<mpq_class>& A, const Vector<mpq_class>& b,
                                      const Vector<mpq_class>& c, internal::Basis<mpq_class>& basis) {
  DELPI_ASSERT(A.rows() == b.size(), "Inconsistent number of rows in A and b");
  DELPI_ASSERT(A.cols() == c.size(), "Inconsistent number of columns in A and c");
  DELPI_ASSERT(basis.size() == A.rows(), "Inconsistent number of rows in A and basis");

  // TODO(tend): Use a more sophisticated maximum number of iterations
  constexpr int max_iterations = 100;
  for (int i = 0; i < max_iterations; ++i) {
    internal::BgLinearSystemSolver<mpq_class> solver{config_};
    solver.Factorise(basis);
    Vector<mpq_class> zb{solver.Solve(b)};
    DELPI_ASSERT((zb.array() >= 0).all(), "All values must be non-negative (feasible)");
    Vector<mpq_class> y{solver.TransposeSolve(c(basis.basis_idxs()))};
    fmt::println("cb: {}\ny: {}", c(basis.basis_idxs()), y);
    // Compute the reduced costs to determine the entering variable or optimality
    Vector<mpq_class> r{c - A.transpose() * y};
    fmt::println("c: {}\nrd: {}", c, A.transpose() * y);
    int r_idx = 0;
    for (; r_idx < r.size(); ++r_idx) {
      if (r(r_idx) < 0) break;
    }
    if (r_idx == r.size()) {
      x_ = Eigen::VectorX<mpq_class>::Zero(A.cols());
      x_(basis.basis_idxs()) = zb;
      return LpResult::OPTIMAL;
    }
    fmt::println("Reduced costs: r: {}\nr [{}] = {}", r, r_idx, r(r_idx));

    // Compute the entering variable or unboundedness
    Vector<mpq_class> d{solver.Solve(A.col(r_idx))};
    mpq_class min_ratio = -1;
    int min_idx = -1;
    for (int d_idx = 0; d_idx < d.size(); ++d_idx) {
      if (d(d_idx) <= 0) continue;
      const mpq_class ratio = zb(d_idx) / d(d_idx);
      if (min_ratio == -1 || ratio < min_ratio) {
        min_ratio = ratio;
        min_idx = d_idx;
      }
    }
    fmt::println("zb: {}\nd: {}\n", zb, d);
    if (min_idx == -1) return LpResult::UNBOUNDED;
    fmt::println("Min ratio: [{}] = {}", min_idx, d(min_idx));

    fmt::println("Updating\n{}\n with leaving = {} ({}), entering = {} from A ({})", basis,
                 basis.basis_vectors().col(min_idx), min_idx, A.col(r_idx), r_idx);
    // Update the basis
    basis.Update(A, min_idx, r_idx);
  }
  DELPI_RUNTIME_ERROR("Maximum number of iterations reached");
}

LpResult DelpiLpSolver::FeasibilityCheck(const Matrix<mpq_class>& A, internal::Basis<mpq_class>& basis) {
  DELPI_ASSERT(A.rows() == b_.size(), "Inconsistent number of rows in A and b");

  DELPI_TRACE("DelpiLpSolver::FeasibilityCheck()");
  std::vector<int> aux_columns{};
  std::vector<int> slack_columns{};
  aux_columns.reserve(A.rows());
  slack_columns.reserve(A.rows());
  for (int i = 0; i < A.rows(); ++i) {
    // If the rhs and the slack variables have different signs or there was no slack variable, add an auxiliary variable
    if ((b_(i) >= 0) != (A(i, i) >= 0) || A(i, i) == 0) {
      aux_columns.push_back(i);
    } else {  // Otherwise, keep using the already existing slack variable
      slack_columns.push_back(i);
    }
  }
  const int num_aux_columns = static_cast<int>(aux_columns.size());

  // No need to run any checks if there are no auxiliary variables. Just set the slack columns as the basis
  if (num_aux_columns == 0) {
    DELPI_TRACE("DelpiLpSolver::FeasibilityCheck(): No auxiliary variables needed");
    basis = internal::Basis{A};
    return LpResult::OPTIMAL;
  }

  //        m n
  // A' = [ I A ] m
  Eigen::MatrixX<mpq_class> aux_A{Eigen::MatrixX<mpq_class>::Zero(A.rows(), num_aux_columns + A.cols())};
  aux_A.rightCols(A.cols()) = A;
  // Set the coefficient of the aux variable to -1 if the rhs is negative, otherwise 1
  for (int i = 0; i < num_aux_columns; ++i) {
    aux_A(aux_columns[i], i) = (b_(aux_columns[i]) < 0) ? -1 : 1;
  }
  DELPI_DEV_FMT("aux_A:\n{}", aux_A);

  //              num_aux_columns         n
  // c' = [ 1 1 .................. 1 0 0 ... 0 ]
  Eigen::VectorX<mpq_class> aux_c{Eigen::VectorX<mpq_class>::Zero(num_aux_columns + A.cols())};
  aux_c.head(num_aux_columns) = Eigen::VectorX<mpq_class>::Ones(num_aux_columns);
  DELPI_DEV_FMT("aux_c:\n{}", aux_c);

  // Initial feasible basis mapping to the columns of A that either contain an aux variable or an active slack variable
  for (int& idx : slack_columns) idx += num_aux_columns;
  std::vector<int> basis_idxs(num_aux_columns);
  std::iota(basis_idxs.begin(), basis_idxs.end(), 0);
  basis_idxs.insert(basis_idxs.end(), slack_columns.begin(), slack_columns.end());
  internal::Basis aux_basis{aux_A, basis_idxs};
  DELPI_DEV_FMT("aux_basis:\n{}", aux_basis);

  // TODO(tend): Solve the feasible problem in increasing precision
  // Solve the auxiliary problem
  const LpResult result = InternalSolve(aux_A, b_, aux_c, aux_basis);
  DELPI_TRACE_FMT("DelpiLpSolver::FeasibilityCheck(): Feasibility check result: {}", result);
  if (result != LpResult::OPTIMAL) return LpResult::INFEASIBLE;
  // Check if the objective value is zero
  const mpq_class objective_value = aux_c.transpose() * x_;
  DELPI_TRACE_FMT("DelpiLpSolver::FeasibilityCheck(): Feasibility check objective value: {}", objective_value);
  if (objective_value != 0) return LpResult::INFEASIBLE;

  RemoveAuxiliaryColumns(aux_A, aux_basis, A.cols());

  basis = aux_basis;
  return LpResult::OPTIMAL;
}
LpResult DelpiLpSolver::OptimalityCheck(const internal::Basis<mpq_class>&) {
  DELPI_TRACE("DelpiLpSolver::OptimalityCheck()");
  return LpResult::OPTIMAL;
}
LpResult DelpiLpSolver::UnboundednessCheck(const internal::Basis<mpq_class>&) {
  DELPI_TRACE("DelpiLpSolver::UnboundednessCheck()");
  return LpResult::OPTIMAL;
}
void DelpiLpSolver::RemoveAuxiliaryColumns(const Matrix<mpq_class>& A, internal::Basis<mpq_class>& basis,
                                           const int num_columns_to_keep) const {
  const int aux_columns = num_columns_to_keep < 0 ? A.cols() - num_columns() : A.cols() - num_columns_to_keep;
  DELPI_ASSERT(aux_columns >= 0, "The number of auxiliary columns must non-negative");

  // Remove the auxiliary variables from the basis
  std::unordered_set<int> aux_basis_idxs{};
  int valid_idx = aux_columns;
  for (int i = 0; i < static_cast<int>(basis.basis_idxs().size()); ++i) {
    if (basis.basis_idxs()[i] >= aux_columns) continue;  // Valid non-auxiliary index
    // Initialise the set only if needed
    if (aux_basis_idxs.empty()) aux_basis_idxs.insert(basis.basis_idxs().begin(), basis.basis_idxs().end());
    // Replace the auxiliary index with any valid index not already in the basis
    for (bool updated = false; valid_idx < A.cols() && !updated; ++valid_idx) {
      if (!aux_basis_idxs.contains(valid_idx)) {
        DELPI_DEV_FMT("Removing auxiliary column: {} ({}) to put {}", i, basis.basis_idxs()[i], valid_idx);
        basis.Update(A, i, valid_idx);
        updated = true;
      }
    }
  }

  // Make all non-auxiliary indices point to the original indices.
  basis.OffsetIndexes(-aux_columns);
  DELPI_ASSERT(
      std::ranges::all_of(basis.basis_idxs(),
                          [num_columns_to_keep](const int idx) { return idx >= 0 && idx < num_columns_to_keep; }),
      "All indices must be valid");
  DELPI_ASSERT(
      std::unordered_set(basis.basis_idxs().begin(), basis.basis_idxs().end()).size() == basis.basis_idxs().size(),
      "All indices must be unique");
}
std::pair<Matrix<mpq_class>, Vector<mpq_class>> DelpiLpSolver::ToSlackForm() const {
  Matrix<mpq_class> slack_A{A_.rows(), A_.cols() + A_.rows()};
  slack_A.rightCols(A_.cols()) = A_;
  slack_A.leftCols(A_.rows()) = Matrix<mpq_class>::Zero(A_.rows(), A_.rows());
  for (int i = 0; i < A_.rows(); ++i) {
    switch (row_senses_[i]) {
      case FormulaKind::Eq:
        break;
      case FormulaKind::Leq:
        slack_A(i, i) = 1;
        break;
      case FormulaKind::Geq:
        slack_A(i, i) = -1;
        break;
      default:
        DELPI_UNREACHABLE();
    }
  }
  Vector<mpq_class> slack_c{Vector<mpq_class>::Zero(A_.cols() + A_.rows())};
  slack_c.tail(c_.size()) = c_;
  return {std::move(slack_A), std::move(slack_c)};
}

std::ostream& operator<<(std::ostream& os, const DelpiLpSolver& solver) {
  return os << "DelpiLpSolver{ A:\n" << solver.A() << ",\nb:\n" << solver.b() << ",\nc:\n" << solver.c() << "\n}";
}

}  // namespace delpi
