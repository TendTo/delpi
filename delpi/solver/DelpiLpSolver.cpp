/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/solver/DelpiLpSolver.h"

#include <ostream>
#include <unordered_set>

#include "delpi/util/error.h"
#include "internal/Basis.h"
#include "internal/BgLinearSystemSolver.h"

namespace delpi {

DelpiLpSolver::DelpiLpSolver(Config config, const std::string& class_name)
    : LpSolver{mpq_class{mpz_class{0}, 0}, mpq_class{mpz_class{0}, 0}, std::move(config), class_name} {}

int DelpiLpSolver::num_columns() const { return static_cast<int>(problem_.num_columns()); }
int DelpiLpSolver::num_rows() const { return static_cast<int>(problem_.num_rows()); }

Column DelpiLpSolver::column(const ColumnIndex column_idx) const {
  DELPI_ASSERT(column_idx < num_columns(), "Column index out of bounds");
  const auto [lb, ub, obj] = problem_.column(column_idx);
  return {col_to_var_.at(column_idx), lb, ub, obj};
}
Row DelpiLpSolver::row(const RowIndex row_idx) const {
  DELPI_ASSERT(row_idx < num_rows(), "Row index out of bounds");
  const Index columns = num_columns();
  const auto [lp_addends, lb, ub]{problem_.row(row_idx)};
  std::vector<std::pair<Variable, mpq_class>> addends;
  addends.reserve(columns);
  for (const auto& [col, coeff] : lp_addends) addends.emplace_back(col_to_var_.at(col), coeff);
  return {addends, lb.has_value() ? lb.value() : ninfinity_, ub.has_value() ? ub.value() : infinity_};
}

void DelpiLpSolver::ReserveColumns(const int num_columns) {
  LpSolver::ReserveColumns(num_columns);
  problem_.Reserve(-1, num_columns);
}
void DelpiLpSolver::ReserveRows(const int num_rows) {
  LpSolver::ReserveRows(num_rows);
  problem_.Reserve(num_rows, -1);
}
LpSolver::ColumnIndex DelpiLpSolver::AddColumn(const Variable& var, const mpq_class& obj, const mpq_class& lb,
                                               const mpq_class& ub) {
  DELPI_ASSERT_FMT(!var_to_col_.contains(var), "Variable '{}' already exists in the LP.", var);
  const ColumnIndex column_idx = num_columns();
  var_to_col_.emplace(var, column_idx);
  col_to_var_.emplace_back(var);
  problem_.AddColumn(obj, lb, ub);
  return column_idx;
}
LpSolver::RowIndex DelpiLpSolver::AddRow(const std::vector<Expression::Addend>& addends, const mpq_class& lb,
                                         const mpq_class& ub) {
  if (gmp::IsInfinity(lb) && gmp::IsInfinity(ub)) {
    DELPI_WARN_FMT("Ignoring unbounded row with addends: {}", addends);
    return -1;
  }
  std::unordered_map<Index, mpq_class> row_lhs;
  row_lhs.reserve(addends.size());
  for (const auto& [var, coeff] : addends) row_lhs.emplace(var_to_col_.at(var), coeff);
  problem_.AddRow(row_lhs, lb, ub);  // TODO(tend): Implement AddRow
  return num_rows() - 1;
}
LpSolver::RowIndex DelpiLpSolver::AddRow(const Expression::Addends& lhs, const FormulaKind sense,
                                         const mpq_class& rhs) {
  DELPI_ASSERT(sense == FormulaKind::Eq || sense == FormulaKind::Leq || sense == FormulaKind::Geq,
               "Only equality, less than or equal, and greater than or equal constraints are supported");
  std::unordered_map<Index, mpq_class> row_lhs;
  row_lhs.reserve(lhs.size());
  for (const auto& [var, coeff] : lhs) row_lhs.emplace(var_to_col_.at(var), coeff);
  problem_.AddRow(row_lhs, sense == FormulaKind::Leq ? ninfinity_ : rhs, sense == FormulaKind::Geq ? infinity_ : rhs);
  return num_rows() - 1;
}
void DelpiLpSolver::SetBound(Variable var, const mpq_class& lb, const mpq_class& ub) {
  DELPI_TRACE_FMT("DelpiLpSolver::SetBound({}, {}, {})", var, lb, ub);
  DELPI_ASSERT(var_to_col_.contains(var), "Variable not found in the LP");
  // TODO(tend): Consider upper and lower bounds
  // problem_.SetColumnBound(0, lb, ub);
}
void DelpiLpSolver::SetCoefficient(const RowIndex row, const ColumnIndex column, const mpq_class& value) {
  DELPI_TRACE_FMT("DelpiLpSolver::SetCoefficient({}, {}, {})", row, column, value);
  DELPI_ASSERT(row < num_rows(), "Row index out of bounds");
  DELPI_ASSERT(column < num_columns(), "Column index out of bounds");
  // problem_.SetCoefficient(row, column, value);
}
void DelpiLpSolver::SetObjective(const int column, const mpq_class& value) {
  DELPI_TRACE_FMT("DelpiLpSolver::SetObjective({}, {})", column, value);
  DELPI_ASSERT(column < num_columns(), "Column index out of bounds");
  problem_.SetObjective(column, value);
}

#ifndef NDEBUG
void DelpiLpSolver::Dump() {
  std::cout << "DelpiLpSolver{ num_columns: " << num_columns() << ", num_rows: " << num_rows() << "\n"
            << problem_ << "}\n";
}
#endif
LpResult DelpiLpSolver::SolveCore(mpq_class&, bool) {
  DELPI_DEV_FMT("Problem matrix\n{}", problem_);
  // Compute the slack and auxiliary variables that will be added to the problem
  // ComputeSlackAndAuxVariables();
  Matrix<mpq_class> slack_A;
  Vector<mpq_class> slack_c;
  Vector<mpq_class> slack_b;
  problem_.SlackForm(slack_A, slack_b, slack_c);
  internal::Basis<mpq_class> slack_basis(slack_A);
  DELPI_DEV_FMT("slack_A:\n{}\nslack_b:\n{}\nslack_c:\n{}\nbasis:\n{}", slack_A, slack_b, slack_c, slack_basis);

  const LpResult feasibility_check = FeasibilityCheck(slack_A, slack_b, slack_basis);
  DELPI_DEV_FMT("Feasibility check: {}. Feasible basis:\n{}\nidxs: {}\nFesible sol: {}", feasibility_check, slack_basis,
                slack_basis.basis_idxs(), x_);
  if (feasibility_check == LpResult::INFEASIBLE) return feasibility_check;
  DELPI_ASSERT(feasibility_check == LpResult::OPTIMAL, "Feasibility check must be optimal");
  DELPI_ASSERT(slack_basis.basis_vectors().determinant() != 0, "Basis matrix must be non-singular");

  Matrix<mpq_class> slack_full_matrix{slack_A.rows(), slack_A.cols() + 1};
  slack_full_matrix << slack_A, slack_b;
  DELPI_DEV_FMT("Slack Full matrix:\n{}", slack_full_matrix);
  const LpResult result = InternalSolve(slack_A, slack_b, slack_c, slack_basis);

  // Drop the slack variables from the solution
  x_ = x_.tail(num_columns()).eval();
  DELPI_DEV_FMT("Result check: {}. Basis:\n{}\nSolution: {}, c: {}", result, slack_basis, x_, problem_.c());
  if (result == LpResult::OPTIMAL) obj_lb_ = obj_ub_ = problem_.c().transpose() * x_;
  solution_ = std::vector<mpq_class>{x_.data(), x_.data() + x_.size()};
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
  DELPI_ASSERT(slack_A.rows() == slack_b.size(), "Inconsistent number of rows in A and b");

  Index min_idx = -1;
  const mpq_class min_val{slack_b.minCoeff(&min_idx)};
  if (min_val >= 0) {
    // TODO(tend): we can just not do this assignment and return the status directly
    slack_basis = internal::Basis<mpq_class>{slack_A};
    return LpResult::OPTIMAL;
  }

  // Initial feasible basis by adding auxiliary variables
  Matrix<mpq_class> aux_A{};
  Vector<mpq_class> aux_c{};
  std::vector<Index> aux_columns{};
  internal::Basis<mpq_class> aux_basis{AuxForm(slack_A, slack_b, aux_A, aux_c, aux_columns)};

  DELPI_DEV_FMT("aux_A:\n{}", aux_A);
  DELPI_DEV_FMT("aux_c:\n{}", aux_c);
  DELPI_DEV_FMT("aux_basis:\n{}", aux_basis);

  // TODO(tend): Solve the feasible problem in increasing precision
  // Solve the auxiliary problem
  const LpResult result = InternalSolve(aux_A, slack_b, aux_c, aux_basis);
  DELPI_TRACE_FMT("DelpiLpSolver::FeasibilityCheck(): Feasibility check result: {}", result);
  if (result != LpResult::OPTIMAL) return LpResult::INFEASIBLE;
  // Check if the objective value is zero
  const mpq_class objective_value = aux_c.transpose() * x_;
  DELPI_TRACE_FMT("DelpiLpSolver::FeasibilityCheck(): Feasibility check objective value: {}", objective_value);
  if (objective_value != 0) return LpResult::INFEASIBLE;

  RemoveAuxiliaryColumns(aux_basis, aux_columns, slack_A, slack_b, slack_basis);
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
void DelpiLpSolver::RemoveAuxiliaryColumns(const internal::Basis<mpq_class>& aux_basis,
                                           const std::vector<Index>& aux_columns, Matrix<mpq_class>& slack_A,
                                           Vector<mpq_class>& slack_b, internal::Basis<mpq_class>& slack_basis) const {
  DELPI_TRACE("DelpiLpSolver::RemoveAuxiliaryColumns()");
  DELPI_ASSERT(&slack_A == &slack_basis.A(), "The basis must be built from the same matrix A");

  std::vector<Index> rows_to_remove{};
  std::vector<std::size_t> columns_to_remove{};
  for (std::size_t i = 0; i < aux_basis.basis_idxs().size(); ++i) {
    if (aux_basis.basis_idxs()[i] < slack_A.cols()) continue;  // Valid non-auxiliary index. Keep it

    // Remove the row from the coefficient matrix
    Index row = aux_columns.at(aux_basis.basis_idxs()[i] - slack_A.cols());
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
internal::Basis<mpq_class> DelpiLpSolver::AuxForm(const Matrix<mpq_class>& slack_A, const Vector<mpq_class>& slack_b,
                                                  Matrix<mpq_class>& aux_A, Vector<mpq_class>& aux_c,
                                                  std::vector<Index>& aux_columns) const {
  DELPI_TRACE("DelpiLpSolver::StdForm()");
  DELPI_ASSERT(aux_columns.empty(), "Auxiliary columns must be empty");
  DELPI_ASSERT(slack_A.rows() == slack_b.size(), "Inconsistent number of rows in A and b");

  aux_columns.clear();
  std::vector<Index> aux_basis_idx{};
  aux_basis_idx.reserve(slack_A.rows());
  aux_columns.reserve(slack_b.size());
  for (Index i = 0; i < slack_b.size(); ++i) {
    if (slack_b(i) < 0) {
      aux_columns.emplace_back(i);
    } else {
      aux_basis_idx.emplace_back(i);  // Add the slack columns which we know are trivially feasible to the basis
    }
  }

  // Initial feasible basis mapping to the columns of A that either contain an aux variable or an active slack variable
  aux_A = Matrix<mpq_class>{slack_A.rows(), slack_A.cols() + static_cast<Index>(aux_columns.size())};
  aux_A.leftCols(slack_A.cols()) = slack_A;
  aux_A.rightCols(aux_columns.size()).setZero();
  for (Index i = 0; i < static_cast<Index>(aux_columns.size()); ++i) {
    aux_A.col(slack_A.cols() + i).setZero();
    aux_A(aux_columns[i], slack_A.cols() + i) = -1;
    aux_basis_idx.emplace_back(slack_A.cols() + i);  // Add the auxiliary columns to the basis
  }

  aux_c = Vector<mpq_class>::Zero(slack_A.cols() + static_cast<Index>(aux_columns.size()));
  aux_c.tail(static_cast<Index>(aux_columns.size())).setConstant(1);

  DELPI_ASSERT(static_cast<Index>(aux_basis_idx.size()) == slack_A.rows(),
               "Inconsistent number of rows in A and basis");
  DELPI_ASSERT(static_cast<Index>(aux_basis_idx.size()) <= slack_A.rows(),
               "Inconsistent number of rows in A and aux columns");

  return {aux_A, std::move(aux_basis_idx)};
}

std::ostream& operator<<(std::ostream& os, const DelpiLpSolver& solver) {
  return os << "DelpiLpSolver{ num_columns: " << solver.num_columns() << ", num_rows: " << solver.num_rows() << "\n"
            << solver.problem() << "}\n";
}

}  // namespace delpi
