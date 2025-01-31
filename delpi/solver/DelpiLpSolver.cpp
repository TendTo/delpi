/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/solver/DelpiLpSolver.h"

#include <ostream>
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
  const Index columns = num_columns();
  std::vector<std::pair<Variable, mpq_class>> addends;
  addends.reserve(columns);
  for (Index i = 0; i < columns; ++i) {
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
LpSolver::ColumnIndex DelpiLpSolver::AddColumn(const Variable& var, const mpq_class& obj, const mpq_class& lb,
                                               const mpq_class& ub) {
  DELPI_ASSERT_FMT(!var_to_col_.contains(var), "Variable '{}' already exists in the LP.", var);
  DELPI_ASSERT(A_.cols() == c_.size(), "Inconsistent number of columns and objective coefficients");
  const ColumnIndex column_idx = num_columns();
  var_to_col_.emplace(var, column_idx);
  col_to_var_.emplace_back(var);
  A_.conservativeResize(Eigen::NoChange, column_idx + 1);
  A_.rightCols(1).setZero();
  c_.conservativeResize(column_idx + 1);
  c_.tail(1).setConstant(obj);
  // TODO(tend): Consider upper and lower bounds in an efficient way
  if (IsInfinity(lb) || lb != 0) DELPI_NOT_IMPLEMENTED();
  if (!IsInfinity(ub)) AddRow({{var, 1}}, FormulaKind::Leq, ub);
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
  Matrix<mpq_class> full_matrix{A_.rows(), A_.cols() + 1};
  full_matrix << A_, b_;
  DELPI_DEV_FMT("Full matrix:\n{}", full_matrix);

  // Compute the slack and auxiliary variables that will be added to the problem
  ComputeSlackAndAuxVariables();
  Matrix<mpq_class> slack_A;
  Vector<mpq_class> slack_c;
  Vector<mpq_class> slack_b{b_};
  SlackForm(slack_A, slack_c);
  internal::Basis<mpq_class> basis(slack_A);
  DELPI_DEV_FMT("slack_A:\n{}\nslack_c:\n{}", slack_A, slack_c);

  const LpResult feasibility_check = FeasibilityCheck(slack_A, slack_b, basis);
  DELPI_DEV_FMT("Feasibility check: {}. Feasible basis:\n{}\nidxs: {}\nFesible sol: {}", feasibility_check, basis,
                basis.basis_idxs(), x_);
  if (feasibility_check == LpResult::INFEASIBLE) return feasibility_check;
  DELPI_ASSERT(feasibility_check == LpResult::OPTIMAL, "Feasibility check must be optimal");
  DELPI_ASSERT(basis.basis_vectors().determinant() != 0, "Basis matrix must be non-singular");

  Matrix<mpq_class> slack_full_matrix{A_.rows(), A_.cols() + 1};
  slack_full_matrix << A_, b_;
  DELPI_DEV_FMT("Slack Full matrix:\n{}", slack_full_matrix);
  const LpResult result = InternalSolve(slack_A, slack_b, slack_c, basis);

  // Drop the slack variables from the solution
  x_ = x_.tail(num_columns()).eval();
  DELPI_DEV_FMT("Result check: {}. Basis:\n{}\nSolution: {}, c: {}", result, basis, x_, c_);
  if (result == LpResult::OPTIMAL) obj_lb_ = obj_ub_ = c_.transpose() * x_;
  solution_ = std::vector(x_.data(), x_.data() + x_.size());
  return result;
}

LpResult DelpiLpSolver::InternalSolve(const Matrix<mpq_class>& A, const Vector<mpq_class>& b,
                                      const Vector<mpq_class>& c, internal::Basis<mpq_class>& basis) {
  DELPI_ASSERT(A.rows() == b.size(), "Inconsistent number of rows in A and b");
  DELPI_ASSERT(A.cols() == c.size(), "Inconsistent number of columns in A and c");
  DELPI_ASSERT(basis.size() == A.rows(), "Inconsistent number of rows in A and basis");
  DELPI_ASSERT(&A == &basis.A(), "Basis must be built from matrix A");
  DELPI_ASSERT(basis.basis_vectors().determinant() != 0, "Basis matrix must be non-singular");

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

    fmt::println("Updating\n{}\n with leaving = {} from basis.col({}), entering = {} from A.col({})", basis,
                 basis.basis_vectors().col(min_idx), min_idx, A.col(r_idx), r_idx);
    // Update the basis
    basis.Update(min_idx, r_idx);
    DELPI_ASSERT(basis.basis_vectors().determinant() != 0, "Basis matrix must be non-singular");
  }
  DELPI_RUNTIME_ERROR("Maximum number of iterations reached");
}

LpResult DelpiLpSolver::FeasibilityCheck(Matrix<mpq_class>& slack_A, Vector<mpq_class>& slack_b,
                                         internal::Basis<mpq_class>& slack_basis) {
  DELPI_TRACE("DelpiLpSolver::FeasibilityCheck()");
  DELPI_ASSERT(&slack_A == &slack_basis.A(), "Basis must be built from matrix A");
  DELPI_ASSERT(slack_A.rows() == b_.size(), "Inconsistent number of rows in A and b");

  // Initial feasible basis mapping to the columns of A that either contain an aux variable or an active slack variable
  Matrix<mpq_class> aux_A;
  Vector<mpq_class> aux_c;
  internal::Basis aux_basis{AuxForm(slack_A, aux_A, aux_c)};
  DELPI_DEV_FMT("aux_A:\n{}", aux_A);
  DELPI_DEV_FMT("aux_c:\n{}", aux_c);
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

  RemoveAuxiliaryColumns(aux_basis, slack_A, slack_b, slack_basis);
  DELPI_ASSERT(slack_basis.basis_vectors().determinant() != 0, "Basis matrix must be non-singular");
  DELPI_ASSERT(slack_basis.basis_vectors().determinant() != 0, "Basis matrix must be non-singular");

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
void DelpiLpSolver::RemoveAuxiliaryColumns(const internal::Basis<mpq_class>& aux_basis, Matrix<mpq_class>& slack_A,
                                           Vector<mpq_class>& slack_b, internal::Basis<mpq_class>& slack_basis) const {
  DELPI_TRACE("DelpiLpSolver::RemoveAuxiliaryColumns()");
  const Index aux_columns = aux_basis.A().cols() - slack_A.cols();
  DELPI_ASSERT(aux_columns >= 0, "The number of auxiliary columns must non-negative");
  DELPI_ASSERT(&slack_A == &slack_basis.A(), "The basis must be built from the same matrix A");

  std::vector<Index> rows_to_remove{};
  std::vector<std::size_t> columns_to_remove{};
  for (std::size_t i = 0; i < aux_basis.basis_idxs().size(); ++i) {
    if (aux_basis.basis_idxs()[i] < slack_A.cols()) continue;  // Valid non-auxiliary index. Keep it

    // Remove the row from the coefficient matrix
    Index row = std::abs(aux_columns_.at(aux_basis.basis_idxs()[i] - slack_A.cols())) - 1;
    rows_to_remove.emplace_back(row);
    columns_to_remove.emplace_back(i);
  }

  // TODO(tend): we may need to keep the order of rows fixed. But for now let's take the more efficient approach
  for (const Index i : rows_to_remove) {
    DELPI_DEV_FMT("Removing row {}", i);
    slack_A.row(i) = slack_A.row(slack_A.rows() - 1);
    slack_A.conservativeResize(slack_A.rows() - 1, Eigen::NoChange);
    slack_b.row(i) = slack_b.row(slack_b.size() - 1);
    slack_b.conservativeResize(slack_b.size() - 1);
  }

  slack_basis.FromBasis(aux_basis, columns_to_remove);
}
internal::Basis<mpq_class> DelpiLpSolver::AuxForm(const Matrix<mpq_class>& slack_A, Matrix<mpq_class>& aux_A,
                                                  Vector<mpq_class>& aux_c) const {
  DELPI_TRACE("DelpiLpSolver::StdForm()");
  const Index num_aux_columns = static_cast<Index>(aux_columns_.size());
  aux_A = Matrix<mpq_class>{slack_A.rows(), slack_A.cols() + num_aux_columns};
  aux_A.leftCols(slack_A.cols()) = slack_A;
  for (Index col = 0; col < num_aux_columns; ++col) {
    const Index i = std::abs(aux_columns_[col]) - 1;
    const bool positive = aux_columns_[col] >= 0;
    aux_A(std::abs(i), slack_A.cols() + col) = (positive) ? 1 : -1;
  }
  aux_c = Vector<mpq_class>{Vector<mpq_class>::Zero(slack_A.cols() + num_aux_columns)};
  aux_c.tail(num_aux_columns) = Vector<mpq_class>::Ones(num_aux_columns);

  std::vector<Index> basis_idx{};
  basis_idx.reserve(aux_A.rows());
  Index aux_idx = 0;
  Index slack_idx = 0;
  for (Index i = 0; i < aux_A.rows(); ++i) {
    if (aux_idx < static_cast<Index>(aux_columns_.size()) && std::abs(aux_columns_[aux_idx]) - 1 == i) {
      basis_idx.push_back(slack_A.cols() + aux_idx);
      ++aux_idx;
      if (std::abs(slack_columns_[slack_idx]) - 1 == i) ++slack_idx;
    } else {
      basis_idx.push_back(A_.cols() + slack_idx);
      ++slack_idx;
    }
  }
  DELPI_ASSERT(aux_idx == num_aux_columns, "All auxiliary columns must be used");
  return {aux_A, basis_idx};
}
void DelpiLpSolver::SlackForm(Matrix<mpq_class>& slack_A, Vector<mpq_class>& slack_c) const {
  DELPI_TRACE("DelpiLpSolver::StdForm()");
  const Index num_slack_columns = static_cast<Index>(slack_columns_.size());
  slack_A = Matrix<mpq_class>{A_.rows(), A_.cols() + num_slack_columns};
  slack_A.leftCols(A_.cols()) = A_;
  for (Index col = 0; col < num_slack_columns; ++col) {
    const Index i = std::abs(slack_columns_[col]) - 1;
    const bool positive = slack_columns_[col] >= 0;
    slack_A(std::abs(i), A_.cols() + col) = (positive) ? 1 : -1;
  }
  slack_c = Vector<mpq_class>{Vector<mpq_class>::Zero(c_.size() + num_slack_columns)};
  slack_c.head(c_.size()) = c_;
}
void DelpiLpSolver::ComputeSlackAndAuxVariables() {
  DELPI_TRACE("DelpiLpSolver::ComputeSlackAndAuxVariables()");

  // Clean and initialise the slack columns vector and auxiliary columns vector
  slack_columns_.clear();
  aux_columns_.clear();
  slack_columns_.reserve(num_rows());
  aux_columns_.reserve(num_rows());

  for (Index i = 0; i < A_.rows(); ++i) {
    switch (row_senses_[i]) {
      case FormulaKind::Eq:  // If the row is already an equality, no slack variable is needed
        aux_columns_.push_back(b_(i) < 0 ? -(i + 1) : i + 1);
        break;
      case FormulaKind::Leq:  // If the row is a <=, add a non-negative slack variable with a positive coefficient
        slack_columns_.push_back(i + 1);
        if (b_(i) < 0) aux_columns_.push_back(-(i + 1));
        break;
      case FormulaKind::Geq:  // If the row is a >=, add a non-negative slack variable with a negative coefficient
        slack_columns_.push_back(-(i + 1));
        if (b_(i) > 0) aux_columns_.push_back(i + 1);
        break;
      default:
        DELPI_UNREACHABLE();
    }
  }
}

std::ostream& operator<<(std::ostream& os, const DelpiLpSolver& solver) {
  return os << "DelpiLpSolver{ A:\n" << solver.A() << ",\nb:\n" << solver.b() << ",\nc:\n" << solver.c() << "\n}";
}

}  // namespace delpi
