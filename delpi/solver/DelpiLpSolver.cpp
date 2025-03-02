/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/solver/DelpiLpSolver.h"

#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "delpi/util/error.h"
#include "internal/Basis.h"
#include "internal/BgLinearSystemSolver.h"

namespace delpi {

struct SolveConfiguration {
  int precision;
  double tolerance;
};

namespace {
std::array precisions{SolveConfiguration{64, 1e-6}, SolveConfiguration{0, 0}};
}

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
void DelpiLpSolver::SetBound([[maybe_unused]] Variable var, [[maybe_unused]] const mpq_class& lb,
                             [[maybe_unused]] const mpq_class& ub) {
  DELPI_TRACE_FMT("DelpiLpSolver::SetBound({}, {}, {})", var, lb, ub);
  DELPI_ASSERT(var_to_col_.contains(var), "Variable not found in the LP");
  // TODO(tend): Consider upper and lower bounds
  // problem_.SetColumnBound(0, lb, ub);
}
void DelpiLpSolver::SetCoefficient([[maybe_unused]] const RowIndex row, [[maybe_unused]] const ColumnIndex column,
                                   [[maybe_unused]] const mpq_class& value) {
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
LpResult DelpiLpSolver::SolveCore() {
  DELPI_DEBUG("DelpiLpSolver::SolveCore()");
  Matrix<mpq_class> slack_A;
  Vector<mpq_class> slack_c;
  Vector<mpq_class> slack_b;
  problem_.SlackForm(slack_A, slack_b, slack_c);
  internal::Basis<mpq_class> slack_basis(slack_A);
  DELPI_DEV("About to check feasibility");

  const LpResult feasibility_check = FeasibilitySolve(slack_A, slack_b, slack_basis);
  DELPI_DEV_FMT("Feasibility check: {}", feasibility_check);
  if (feasibility_check == LpResult::INFEASIBLE) return feasibility_check;
  DELPI_ASSERT(feasibility_check == LpResult::OPTIMAL, "Feasibility check must be optimal");
  DELPI_ASSERT(slack_basis.basis_vectors().determinant() != 0, "Basis matrix must be non-singular");

  const LpResult optimality_check = OptimalitySolve(slack_A, slack_b, slack_c, slack_basis);

  DELPI_DEV_FMT("Result: {}. x: {}, c: {}", optimality_check, x_, problem_.c());
  solution_ = std::vector<mpq_class>{x_.data(), x_.data() + x_.size()};
  return optimality_check;
}

template <IsAnyOf<double, mpq_class> T>
LpResult DelpiLpSolver::InternalSolve(const Matrix<T>& A, const Vector<T>& b, const Vector<T>& c, const T& tolerance,
                                      internal::Basis<T>& basis, Vector<T>* const x, T* const obj) {
  if constexpr (std::is_same_v<T, mpq_class>) {
    DELPI_ASSERT(tolerance == 0, "Tolerance must be 0 for exact arithmetic");
  }

  DELPI_ASSERT(A.rows() == b.size(), "Inconsistent number of rows in A and b");
  DELPI_ASSERT(A.cols() == c.size(), "Inconsistent number of columns in A and c");
  DELPI_ASSERT(basis.size() == A.rows(), "Inconsistent number of rows in A and basis");
  DELPI_ASSERT(&A == &basis.A(), "Basis must be built from matrix A");
  DELPI_ASSERT(basis.basis_vectors().determinant() != 0, "Basis matrix must be non-singular");

  // TODO(tend): Use a more sophisticated maximum number of iterations
  constexpr int max_iterations = 1000;
  for (int i = 0; i < max_iterations; ++i) {
    internal::BgLinearSystemSolver<T> solver{config_};
    solver.Factorise(basis);
    Vector<T> zb{solver.Solve(b)};
    DELPI_ASSERT((zb.array() >= -tolerance).all(), "All values must be non-negative (feasible)");
    Vector<T> y = solver.TransposeSolve(c(basis.basis_idxs()));
    // fmt::println("cb: {}\ny: {}", c(basis.basis_idxs()), y);
    // Compute the reduced costs to determine the entering variable or optimality
    Vector<T> r{c - A.transpose() * y};
    // fmt::println("c: {}\nrd: {}", c, A.transpose() * y);
    int r_idx = 0;
    for (; r_idx < r.size(); ++r_idx) {
      if (r(r_idx) < -tolerance) break;
    }
    if (r_idx == r.size()) {
      if (nullptr != x || nullptr != obj) {
        Vector<T> _x = Eigen::VectorX<T>::Zero(A.cols());
        _x(basis.basis_idxs()) = solver.Solve(b);
        if (x != nullptr) *x = _x;
        if (obj != nullptr) *obj = c.transpose() * _x;
      }
      // fmt::println("b: {}, index: {}, x_: {}", b, basis.basis_idxs(), x_);
      return LpResult::OPTIMAL;
    }
    // fmt::println("Reduced costs: r: {}\nr [{}] = {}", r, r_idx, r(r_idx));

    // Compute the entering variable or unboundedness
    Vector<T> d{solver.Solve(A.col(r_idx))};
    mpq_class min_ratio = -1;
    int min_idx = -1;
    for (int d_idx = 0; d_idx < d.size(); ++d_idx) {
      if (d(d_idx) <= tolerance) continue;
      const mpq_class ratio = zb(d_idx) / d(d_idx);
      if (min_ratio == -1 || ratio < min_ratio) {
        min_ratio = ratio;
        min_idx = d_idx;
      }
    }
    // fmt::println("zb: {}\nd: {}\n", zb, d);
    if (min_idx == -1) return LpResult::UNBOUNDED;
    // fmt::println("Min ratio: [{}] = {}", min_idx, d(min_idx));
    //
    // fmt::println("Updating\n{}\n with leaving = {} from basis.col({}), entering = {} from A.col({})", basis,
    //              basis.basis_vectors().col(min_idx), min_idx, A.col(r_idx), r_idx);
    fmt::println("Leaving = {}, entering = {}", min_idx, r_idx);

    // Update the basis
    basis.Update(min_idx, r_idx);
    // DELPI_ASSERT(basis.basis_vectors().determinant() != 0, "Basis matrix must be non-singular");
  }
  DELPI_RUNTIME_ERROR("Maximum number of iterations reached");
}

LpResult DelpiLpSolver::FeasibilitySolve(Matrix<mpq_class>& slack_A, Vector<mpq_class>& slack_b,
                                         internal::Basis<mpq_class>& slack_basis) {
  DELPI_TRACE("DelpiLpSolver::FeasibilitySolve()");
  DELPI_ASSERT(&slack_A == &slack_basis.A(), "Basis must be built from matrix A");
  DELPI_ASSERT(slack_A.rows() == slack_b.size(), "Inconsistent number of rows in A and b");

  // Initial feasible basis by adding auxiliary variables
  Matrix<mpq_class> aux_A{};
  Vector<mpq_class> aux_c{};
  std::vector<Index> aux_columns{};
  internal::Basis<mpq_class> aux_basis{AuxForm(slack_A, slack_b, aux_A, aux_c, aux_columns)};

  DELPI_ASSERT(!aux_basis.basis_idxs().empty(), "Auxiliary basis must have at least one index");
  if (*std::ranges::max_element(aux_basis.basis_idxs()) < slack_A.cols()) {
    // TODO(tend): we can just not do this assignment and return the status directly
    slack_basis = internal::Basis<mpq_class>{slack_A};
    return LpResult::OPTIMAL;
  }

  // TODO(tend): Solve the feasible problem in increasing precision
  for (const auto& [precision, tolerance] : precisions) {
    LpResult feas_result;
    double feas_obj;
    internal::Basis<mpq_class> feas_basis{aux_A, aux_basis};

    if (precision == 0) {
      DELPI_UNREACHABLE();
      DELPI_DEBUG("Feasibility check with precision=0 (rational)");
      mpq_class obj;
      feas_result = InternalSolve(aux_A, slack_b, aux_c, mpq_class{0}, feas_basis,
                                  static_cast<Vector<mpq_class>*>(nullptr), &obj);
      feas_obj = obj.get_d();
    } else if (precision == 64) {  // double precision
      DELPI_DEBUG("Feasibility check with precision=64 (double)");
      Matrix<double> aux_A_d = aux_A.cast<double>();
      Vector<double> slack_b_d = slack_b.cast<double>();
      Vector<double> aux_c_d = aux_c.cast<double>();
      // DELPI_DEV_FMT("A:\n{}\nb:\n{}\nc:\n{}", aux_A_d, slack_b_d, aux_c_d);
      internal::Basis<double> aux_basis_d{aux_A_d, aux_basis};
      feas_result = InternalSolve(aux_A_d, slack_b_d, aux_c_d, 1e-6, aux_basis_d, static_cast<Vector<double>*>(nullptr),
                                  &feas_obj);
      feas_basis = aux_basis_d;
    } else {
      DELPI_UNREACHABLE();
    }

    DELPI_DEV_FMT("Feasibility check: result={}, obj={}, tolerance={}", feas_result, feas_obj, tolerance);
    // The auxiliary problem is always feasible and bounded. We need to investigate further
    if (feas_result != LpResult::OPTIMAL) continue;
    // The objective value is less than the tolerance. We can say that the problem is feasible
    if (feas_obj <= tolerance) {
      RemoveAuxiliaryColumns(feas_basis, aux_columns, slack_A, slack_b, slack_basis);
      return LpResult::OPTIMAL;
    }

    // The floating point simplex returned optimal with an obj value > tolerance.
    // We need to certify the feasibility of the problem with exact arithmetic
    const LpResult feasibility = FeasibilityCheck(aux_A, slack_b, aux_c, feas_basis);
    if (feasibility == LpResult::INFEASIBLE) return LpResult::INFEASIBLE;
    // We have found a feasible solution, remove the auxiliary columns and return the result
    if (feasibility == LpResult::OPTIMAL) {
      RemoveAuxiliaryColumns(feas_basis, aux_columns, slack_A, slack_b, slack_basis);
      return LpResult::OPTIMAL;
    }
  }
  throw DelpiLpSolverException("Could not prove feasibility with the provided precisions");
}
LpResult DelpiLpSolver::OptimalitySolve(const Matrix<mpq_class>& slack_A, const Vector<mpq_class>& slack_b,
                                        const Vector<mpq_class>& slack_c, internal::Basis<mpq_class>& slack_basis) {
  DELPI_TRACE("DelpiLpSolver::OptimalitySolve()");
  DELPI_ASSERT(&slack_A == &slack_basis.A(), "Basis must be built from matrix A");
  DELPI_ASSERT(slack_A.rows() == slack_b.size(), "Inconsistent number of rows in A and b");

  // TODO(tend): Solve the feasible problem in increasing precision
  for (const auto& [precision, tolerance] : precisions) {
    LpResult opt_result;
    internal::Basis<mpq_class> opt_basis{slack_A, slack_basis};

    if (precision == 0) {
      DELPI_UNREACHABLE();
      DELPI_DEBUG("Feasibility check with precision=0 (rational)");
      mpq_class obj;
      opt_result = InternalSolve(slack_A, slack_b, slack_c, mpq_class{0}, opt_basis);
    } else if (precision == 64) {  // double precision
      DELPI_DEBUG("Feasibility check with precision=64 (double)");
      Matrix<double> slack_A_d = slack_A.cast<double>();
      Vector<double> slack_b_d = slack_b.cast<double>();
      Vector<double> slack_c_d = slack_c.cast<double>();
      // DELPI_DEV_FMT("A:\n{}\nb:\n{}\nc:\n{}", slack_A_d, slack_b_d, slack_c_d);
      internal::Basis<double> aux_basis_d{slack_A_d, slack_basis};
      opt_result = InternalSolve(slack_A_d, slack_b_d, slack_c_d, 1e-6, aux_basis_d);
      opt_basis = aux_basis_d;
    } else {
      DELPI_UNREACHABLE();
    }

    DELPI_DEV_FMT("Optimality check: result={} tolerance={}", opt_result, tolerance);
    // The optimality problem is always feasible. We need to investigate further
    if (opt_result == LpResult::INFEASIBLE) continue;

    // The floating point simplex returned unbounded. We need to certify the unboundedness of the problem
    if (opt_result == LpResult::UNBOUNDED) {
      if (UnboundednessCheck(slack_A, slack_b, slack_c, opt_basis) == LpResult::UNBOUNDED) {
        slack_basis = opt_basis;
        return LpResult::UNBOUNDED;
      }
    }

    // The floating point simplex returned optimal. We need to certify the optimality of the problem
    if (opt_result == LpResult::OPTIMAL) {
      if (OptimalityCheck(slack_A, slack_b, slack_c, opt_basis) == LpResult::OPTIMAL) {
        slack_basis = opt_basis;
        return LpResult::OPTIMAL;
      }
    }
  }
  throw DelpiLpSolverException("Could not find an optimal solution with the provided precisions");
}

LpResult DelpiLpSolver::FeasibilityCheck(const Matrix<mpq_class>& aux_A, const Vector<mpq_class>& slack_b,
                                         const Vector<mpq_class>& aux_c,
                                         const internal::Basis<mpq_class>& feas_basis) const {
  DELPI_ASSERT(aux_A.rows() == slack_b.size(), "Inconsistent number of rows in A and b");
  DELPI_ASSERT(&aux_A == &feas_basis.A(), "Basis must be built from matrix A");
  DELPI_ASSERT(feas_basis.basis_vectors().determinant() != 0, "Basis matrix must be non-singular");

  DELPI_DEV("Feasibility check: auxiliary problem is feasible and bounded");
  if (feas_basis.basis_vectors().determinant() == 0) return LpResult::ERROR;
  internal::BgLinearSystemSolver<mpq_class> solver{config_};
  solver.Factorise(feas_basis);
  const Vector<mpq_class> zb{solver.Solve(slack_b)};
  const mpq_class obj{aux_c(feas_basis.basis_idxs()).transpose() * zb};
  const Vector<mpq_class> y{solver.TransposeSolve(aux_c(feas_basis.basis_idxs()))};
  const Vector<mpq_class> r{aux_c - aux_A.transpose() * y};
  DELPI_DEV_FMT("Feasibility check: zb>=0 ? {} | r>=0 ? {} | obj={}", (zb.array() >= 0).all(), (r.array() >= 0).all(),
                obj);
  if ((zb.array() >= 0).all() && (r.array() >= 0).all() && obj > 0) return LpResult::INFEASIBLE;
  return LpResult::OPTIMAL;  // TODO(tend): return ERROR?
}
LpResult DelpiLpSolver::OptimalityCheck(const Matrix<mpq_class>& slack_A, const Vector<mpq_class>& slack_b,
                                        const Vector<mpq_class>& slack_c, const internal::Basis<mpq_class>& basis) {
  DELPI_TRACE("DelpiLpSolver::OptimalityCheck()");
  if (basis.basis_vectors().determinant() == 0) return LpResult::ERROR;
  internal::BgLinearSystemSolver<mpq_class> solver{config_};
  solver.Factorise(basis);
  const Vector<mpq_class> zb{solver.Solve(slack_b)};
  // Primal infeasible
  if ((zb.array() < 0).any()) return LpResult::ERROR;
  const Vector<mpq_class> y{solver.TransposeSolve(slack_c(basis.basis_idxs()))};
  const Vector<mpq_class> r{slack_c - slack_A.transpose() * y};
  // Dual infeasible
  if ((r.array() < 0).any()) return LpResult::ERROR;

  obj_lb_ = slack_b.transpose() * y;
  obj_ub_ = slack_c(basis.basis_idxs()).transpose() * zb;
  delta_ = obj_ub_ - obj_lb_;
  if (delta_ > config_.delta()) return LpResult::ERROR;

  // Compute the original problem solution
  x_ = Vector<mpq_class>::Zero(slack_A.cols());
  x_(basis.basis_idxs()) = zb;
  problem_.FixSolution(x_);
  obj_ub_ = slack_c.head(problem_.num_columns()).transpose() * x_;
  obj_lb_ = obj_ub_ - delta_;

  return LpResult::OPTIMAL;
}
LpResult DelpiLpSolver::UnboundednessCheck(const Matrix<mpq_class>& slack_A, const Vector<mpq_class>& slack_b,
                                           const Vector<mpq_class>& slack_c,
                                           const internal::Basis<mpq_class>& basis) const {
  internal::BgLinearSystemSolver<mpq_class> solver{config_};
  solver.Factorise(basis);
  const Vector<mpq_class> zb{solver.Solve(slack_b)};
  if ((zb.array() < 0).any()) return LpResult::ERROR;
  const auto y{solver.TransposeSolve(slack_c(basis.basis_idxs()))};
  const Vector<mpq_class> r{slack_c - slack_A.transpose() * y};
  for (Index i = 0; i < r.size(); ++i) {
    if (r(i) < 0) {
      const Vector<mpq_class> d{solver.Solve(slack_A.col(i))};
      if ((d.array() <= 0).all()) return LpResult::UNBOUNDED;
    }
  }
  return LpResult::ERROR;
}
void DelpiLpSolver::RemoveAuxiliaryColumns(const internal::Basis<mpq_class>& feas_basis,
                                           const std::vector<Index>& aux_columns, Matrix<mpq_class>& slack_A,
                                           Vector<mpq_class>& slack_b, internal::Basis<mpq_class>& slack_basis) const {
  DELPI_TRACE("DelpiLpSolver::RemoveAuxiliaryColumns()");
  DELPI_ASSERT(&slack_A == &slack_basis.A(), "The basis must be built from the same matrix A");

  std::vector<Index> rows_to_remove{};
  std::vector<std::size_t> columns_to_remove{};
  for (std::size_t i = 0; i < feas_basis.basis_idxs().size(); ++i) {
    if (feas_basis.basis_idxs()[i] < slack_A.cols()) continue;  // Valid non-auxiliary index. Keep it

    // Remove the row from the coefficient matrix
    Index row = aux_columns.at(feas_basis.basis_idxs()[i] - slack_A.cols());
    rows_to_remove.emplace_back(row);
    columns_to_remove.emplace_back(i);
  }

  // TODO(tend): we may need to keep the order of rows fixed. But for now let's take the more efficient approach
  for (const Index i : rows_to_remove) {
    DELPI_DEV_FMT("Removing row {}", i);
    slack_A.row(i) = slack_A.row(slack_A.rows() - 1).eval();
    slack_A.conservativeResize(slack_A.rows() - 1, Eigen::NoChange);
    slack_b.row(i) = slack_b.row(slack_b.size() - 1).eval();
    slack_b.conservativeResize(slack_b.size() - 1);
  }

  slack_basis.FromBasis(feas_basis, columns_to_remove);
  DELPI_ASSERT(slack_basis.basis_vectors().determinant() != 0, "Basis matrix must be non-singular");
}
internal::Basis<mpq_class> DelpiLpSolver::AuxForm(const Matrix<mpq_class>& slack_A, const Vector<mpq_class>& slack_b,
                                                  Matrix<mpq_class>& aux_A, Vector<mpq_class>& aux_c,
                                                  std::vector<Index>& aux_columns) const {
  DELPI_TRACE("DelpiLpSolver::StdForm()");
  DELPI_ASSERT(aux_columns.empty(), "Auxiliary columns must be empty");
  DELPI_ASSERT(slack_A.rows() == slack_b.size(), "Inconsistent number of rows in A and b");

  for (Index i = 0; i < slack_A.rows(); ++i) {
    aux_columns.emplace_back(i);
  }

  aux_A = Matrix<mpq_class>{slack_A.rows(), slack_A.cols() + slack_b.size()};
  aux_A.leftCols(slack_A.cols()) = slack_A;
  aux_A.rightCols(slack_b.size()).setZero();

  std::vector<Index> aux_basis_idx{};
  aux_basis_idx.reserve(slack_A.rows());

  for (Index i = 0; i < slack_b.size(); ++i) {
    if (slack_A(i, slack_A.cols() - slack_A.rows() + i) != 0 &&
        (slack_b(i) < 0) == (slack_A(i, slack_A.cols() - slack_A.rows() + i) < 0)) {
      aux_basis_idx.emplace_back(slack_A.cols() - slack_A.rows() + i);
    } else {
      aux_A(i, slack_A.cols() + i) = slack_b(i) < 0 ? -1 : 1;
      aux_basis_idx.emplace_back(slack_A.cols() + i);
    }
  }

  aux_c = Vector<mpq_class>::Zero(slack_A.cols() + slack_b.size());
  aux_c.tail(slack_b.size()).setConstant(1);

  // More efficient implementation which only adds the necessary aux columns
#if 0
  aux_columns.clear();
  std::vector<Index> aux_basis_idx{};
  aux_basis_idx.reserve(slack_A.rows());
  aux_columns.reserve(slack_b.size());
  for (Index i = 0; i < slack_b.size(); ++i) {
    if ((slack_b(i) < 0) != slack_A(i, slack_A.cols() - slack_A.rows()) < 0) {
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
#endif

  return {aux_A, std::move(aux_basis_idx)};
}

std::ostream& operator<<(std::ostream& os, const DelpiLpSolver& solver) {
  return os << "DelpiLpSolver{ num_columns: " << solver.num_columns() << ", num_rows: " << solver.num_rows() << "\n"
            << solver.problem() << "}\n";
}

template LpResult DelpiLpSolver::InternalSolve(const Matrix<double>&, const Vector<double>&, const Vector<double>&,
                                               const double&, internal::Basis<double>&, Vector<double>*, double*);
template LpResult DelpiLpSolver::InternalSolve(const Matrix<mpq_class>&, const Vector<mpq_class>&,
                                               const Vector<mpq_class>&, const mpq_class&, internal::Basis<mpq_class>&,
                                               Vector<mpq_class>*, mpq_class*);

}  // namespace delpi
