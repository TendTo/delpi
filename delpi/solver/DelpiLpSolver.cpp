/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/solver/DelpiLpSolver.h"

#include <iostream>
#include <ostream>

#include "delpi/util/error.h"
#include "internal/Basis.h"
#include "internal/BgLinearSystemSolver.h"

namespace delpi {

template <>
LpResult DelpiLpSolver::LpSolve(const Matrix<mpq_class>& A, const Vector<mpq_class>& b, const Vector<mpq_class>& c,
                                internal::Basis<mpq_class>& basis) {
  // TODO(tend): Use a more sophisticated maximum number of iterations
  constexpr int max_iterations = 5;
  for (int i = 0; i < max_iterations; ++i) {
    internal::BgLinearSystemSolver<mpq_class> solver{config_};
    solver.Factorise(basis);
    Vector<mpq_class> zb{solver.Solve(b)};
    Vector<mpq_class> y{solver.TransposeSolve(c(basis.basis_idxs()))};
    // Compute the reduced costs to determine the entering variable or optimality
    Vector<mpq_class> r{c - A.transpose() * y};
    int r_idx = 0;
    for (; r_idx < r.size(); ++r_idx) {
      if (r(r_idx) < 0) break;
    }
    if (r_idx == r.size()) {
      x_ = Eigen::VectorX<mpq_class>::Zero(A.cols());
      x_(basis.basis_idxs()) = zb;
      return LpResult::OPTIMAL;
    }
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
    if (min_idx == -1) return LpResult::UNBOUNDED;

    fmt::println("Updating\n{}\n with leaving = {}, entering = {}", basis, min_idx, r_idx);
    // Update the basis
    basis.Update(A, min_idx, r_idx);
  }
  DELPI_RUNTIME_ERROR("Maximum number of iterations reached");
}

DelpiLpSolver::DelpiLpSolver(Config config, const std::string& class_name)
    : LpSolver{mpq_class{std::numeric_limits<long>::min()}, mpq_class{std::numeric_limits<long>::max()},
               std::move(config), class_name} {}

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
  return {addends, b_(row_idx), b_(row_idx)};
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
  // TODO(tend): Support multiple constraint senses
  if (lb != ub) throw DelpiNotImplementedException("Only equality constraints are supported");
  Expression::Addends map_addends;
  for (const auto& [var, coeff] : addends) map_addends.emplace(var, coeff);
  return AddRow(map_addends, FormulaKind::Eq, lb);
}
LpSolver::RowIndex DelpiLpSolver::AddRow(const Expression::Addends& lhs, const FormulaKind sense,
                                         const mpq_class& rhs) {
  // TODO(tend): Support multiple constraint senses
  if (sense != FormulaKind::Eq) throw DelpiNotImplementedException("Only equality constraints are supported");
  const RowIndex row_idx = num_rows();
  // Set the coefficients in the A matrix
  A_.conservativeResize(row_idx + 1, Eigen::NoChange);
  A_.bottomRows(1).setZero();
  for (const auto& [var, coeff] : lhs) A_(row_idx, var_to_col_.at(var)) = coeff;
  // Set the right-hand side in the b vector
  b_.conservativeResize(row_idx + 1);
  b_.tail(1).setConstant(rhs);
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
  std::cout << "DelpiLpSolver{ A: " << A_ << ",\nb: " << b_ << ",\nc: " << c_ << " }" << std::endl;
}
#endif
LpResult DelpiLpSolver::SolveCore(mpq_class&, bool) {
  internal::Basis<mpq_class> basis{A_};
  const LpResult result = LpSolve(A_, b_, c_, basis);
  obj_lb_ = obj_ub_ = c_.transpose() * x_;
  return result;
  DELPI_UNREACHABLE();
#if 0
  const int m = num_rows();
  // TODO: change row and column indices based on the current situation
  const auto cb = c_.tail(m);
  const auto cn = c_.head(c_.size() - m);
  B_ = A_.rightCols(m);
  std::cout << "B:\n" << B_ << std::endl;
  const auto LU = B_.fullPivLu();
  L_ = LU.matrixLU().triangularView<Eigen::StrictlyLower>();
  U_ = LU.matrixLU().triangularView<Eigen::Upper>();
  std::cout << "L:\n" << L_ << std::endl;
  std::cout << "U:\n" << U_ << std::endl;
  // 1. B x = b
  x_ = LU.solve(b_);
  std::cout << "1. x:\n" << x_ << std::endl;
  // 2. B^T pi = c_B
  const Eigen::VectorX<mpq_class> pi{LU.transpose().solve(cb)};
  std::cout << "2. pi:\n" << pi << std::endl;
  // 3. C = cn - pi^T A_n
  const Eigen::VectorX<mpq_class> reduced_cost{cn.transpose() - pi.transpose() * (A_.leftCols(A_.cols() - m))};
  std::cout << "3. reduced_cost:\n" << reduced_cost.matrix() << std::endl;
  // 4. Select the most negative reduced cost
  int min_index = -1;
  const mpq_class& min_value = reduced_cost.minCoeff(&min_index);
  std::cout << "4. min_index: " << min_index << ", min_value: " << min_value << std::endl;
  // 4.1. If all reduced costs are non-negative, the solution is optimal
  if (min_value >= 0) return LpResult::OPTIMAL;
  // 4.2. Otherwise, we have found the entering variable at min_index
  std::cout << "4.2. entering var: " << col_to_var_.at(min_index) << std::endl;

  // 5. B y = A_min
  const Eigen::VectorX<mpq_class> A_min{LU.solve(A_.col(min_index))};
  std::cout << "5. A_min:\n" << A_min << std::endl;
  // 6.1. If all elements of A_min are non-positive, the problem is unbounded
  if (A_min.maxCoeff() <= 0) return LpResult::UNBOUNDED;
  // 6.1 Otherwise, we have found the leaving variable
  int min_ratio_index = -1;
  const Eigen::ArrayX<mpq_class> ratio{x_.array() / A_min.array()};
  const mpq_class& min = ratio.minCoeff(&min_ratio_index);
  std::cout << "6.1. min_ratio_index: " << min_ratio_index << ", min_ratio: " << min << std::endl;

  return LpResult::ERROR;
#endif
}

LpResult DelpiLpSolver::FeasibilityCheck() {
  // Check if the current solution is feasible

  //              m         n
  // c' = [ 1 1 1 ... 1 0 0 ... 0 ]
  Eigen::VectorX<mpq_class> aux_c{Eigen::VectorX<mpq_class>::Zero(num_columns() + num_rows())};
  aux_c(0, Eigen::seqN(0, num_rows())) = Eigen::VectorX<mpq_class>::Ones(num_rows());
  DELPI_DEV_FMT("aux_c:\n{}", aux_c);

  // A' = [ I A ]
  Eigen::MatrixX<mpq_class> aux_A{Eigen::MatrixX<mpq_class>::Zero(num_rows(), num_columns() + num_rows())};
  aux_A(Eigen::all, Eigen::seqN(0, num_rows())) = Eigen::MatrixX<mpq_class>::Identity(num_rows(), num_rows());
  aux_A(Eigen::all, Eigen::seqN(num_rows(), num_columns())) = A_;
  // Set the diagonal value to -1 if the rhs is negative
  for (int i = 0; i < num_rows(); ++i) {
    if (b_(i) < 0) aux_A(i, i) = -1;
  }
  DELPI_DEV_FMT("aux_A:\n{}", aux_A);

  // Initial feasible basis
  internal::Basis aux_basis{aux_A};
  DELPI_DEV_FMT("aux_basis:\n{}", aux_basis);

  // TODO(tend): Solve the feasible problem

  return LpResult::OPTIMAL;
}
LpResult DelpiLpSolver::OptimalityCheck() { return LpResult::OPTIMAL; }
LpResult DelpiLpSolver::UnboundednessCheck() { return LpResult::OPTIMAL; }

std::ostream& operator<<(std::ostream& os, const DelpiLpSolver& solver) {
  return os << "DelpiLpSolver{ A:\n" << solver.A() << ",\nb:\n" << solver.b() << ",\nc:\n" << solver.c() << "\n}";
}

}  // namespace delpi
