#include "delpi/solver/internal/LpProblem.h"

#include "delpi/util/error.h"
#include "delpi/util/logging.h"

namespace delpi::internal {

Row LpProblem::row(const Index row_idx) const {
  DELPI_ASSERT(0 <= row_idx && row_idx < num_rows_, "Row index out of bounds");
  std::vector<std::pair<Index, T>> addends;
  addends.reserve(num_columns_);
  for (Index i = 0; i < num_columns_; ++i) {
    if (A_.coeff(row_idx, i) != 0) addends.emplace_back(i, A_.coeff(row_idx, i));
  }
  return {addends, sense_[row_idx] != FormulaKind::Leq ? std::optional<mpq_class>{b_(row_idx)} : std::nullopt,
          sense_[row_idx] != FormulaKind::Geq ? std::optional<mpq_class>{b_(row_idx)} : std::nullopt};
}
Column LpProblem::column(const Index column_idx) const {
  DELPI_ASSERT(0 <= column_idx && column_idx < num_columns_, "Column index out of bounds");
  return {gmp::IsInfinity(x_lb_.coeff(column_idx)) ? std::nullopt : std::optional<mpq_class>{x_lb_.coeff(column_idx)},
          x_ub_.contains(column_idx) ? std::optional<mpq_class>{x_ub_.at(column_idx)} : std::nullopt,
          c_(column_idx) == 0 ? std::nullopt : std::optional<mpq_class>{c_(column_idx)}};
}
std::vector<Row> LpProblem::rows() const {
  std::vector<Row> result;
  result.reserve(num_rows_);
  for (Index i = 0; i < num_rows_; ++i) result.push_back(row(i));
  return result;
}
std::vector<Column> LpProblem::columns() const {
  std::vector<Column> result;
  result.reserve(num_columns_);
  for (Index i = 0; i < num_columns_; ++i) result.push_back(column(i));
  return result;
}
void LpProblem::AddColumn(const T& obj, const T& lb, const T& ub) {
  DELPI_TRACE_FMT("LpProblem::AddColumn({}, {}, {})", obj, lb, ub);
  DELPI_ASSERT(!gmp::IsInfinity(lb) || !gmp::IsInfinity(ub), "Free variables are not supported");
  const Index column_idx = num_columns_;
  DELPI_TRACE_FMT("LpProblem::AddColumn: adding new variable at column {}", column_idx);
  // Coefficient matrix
  if (A_.cols() < column_idx + 1) A_.conservativeResize(Eigen::NoChange, column_idx + 1);
  // Objective function
  if (c_.size() < column_idx + 1) c_.conservativeResize(column_idx + 1);
  c_(column_idx) = obj;
  // Lower bound
  if (x_lb_.size() < column_idx + 1) x_lb_.conservativeResize(column_idx + 1);
  x_lb_.insert(column_idx) = lb;
  // Upper bound
  if (!gmp::IsInfinity(ub)) x_ub_.emplace(column_idx, ub);
  num_columns_++;
  DELPI_ASSERT(A_.cols() == c_.size(), "Inconsistent number of columns and objective coefficients");
  DELPI_ASSERT(A_.cols() == x_lb_.size(), "Inconsistent number of columns and lower bounds");
  DELPI_ASSERT(x_ub_.size() <= static_cast<std::size_t>(A_.cols()), "Inconsistent number of columns and upper bounds");
}
void LpProblem::AddRow(const std::unordered_map<Index, T>& row, const T& lb, const T& ub) {
  DELPI_TRACE_FMT("LpProblem::AddRow({}, {}, {})", row, lb, ub);
  // No need to consider unbounded rows
  if (gmp::IsInfinity(lb) && gmp::IsInfinity(ub)) return;
  if (!gmp::IsInfinity(lb) && !gmp::IsInfinity(ub) && lb == ub) {
    const Index row_idx = num_rows_;
    DELPI_TRACE_FMT("LpProblem::AddRow: adding new row at index {}", row_idx);
    // Set the coefficients in the A matrix
    if (A_.rows() < row_idx + 1) A_.conservativeResize(row_idx + 1, num_columns_);
    for (const auto& [idx, coeff] : row) A_.insert(row_idx, idx) = coeff;
    if (b_.size() < row_idx + 1) b_.conservativeResize(row_idx + 1);
    b_(row_idx) = lb;
    sense_.push_back(FormulaKind::Eq);
    num_rows_++;
    return;
  }
  if (!gmp::IsInfinity(lb)) {
    const Index row_idx = num_rows_;
    DELPI_TRACE_FMT("LpProblem::AddRow: adding new row at index {}", row_idx);
    // Set the coefficients in the A matrix
    if (A_.rows() < row_idx + 1) A_.conservativeResize(row_idx + 1, num_columns_);
    for (const auto& [idx, coeff] : row) A_.insert(row_idx, idx) = coeff;
    if (b_.size() < row_idx + 1) b_.conservativeResize(row_idx + 1);
    b_(row_idx) = lb;
    sense_.push_back(FormulaKind::Geq);
    num_rows_++;
  }
  if (!gmp::IsInfinity(ub)) {
    const Index row_idx = num_rows_;
    DELPI_TRACE_FMT("LpProblem::AddRow: adding new row at index {}", row_idx);
    // Set the coefficients in the A matrix
    if (A_.rows() < row_idx + 1) A_.conservativeResize(row_idx + 1, num_columns_);
    for (const auto& [idx, coeff] : row) A_.insert(row_idx, idx) = coeff;
    if (b_.size() < row_idx + 1) b_.conservativeResize(row_idx + 1);
    b_(row_idx) = ub;
    sense_.push_back(FormulaKind::Leq);
    num_rows_++;
  }
}
void LpProblem::SetObjective(Index column_idx, const T& value) {
  DELPI_TRACE_FMT("LpProblem::SetObjective({}, {})", column_idx, value);
  DELPI_ASSERT(0 <= column_idx && column_idx < num_columns_, "Column index out of bounds");
  c_(column_idx) = value;
}
void LpProblem::Reserve(const Index num_rows, const Index num_columns) {
  DELPI_TRACE_FMT("LpProblem::Reserve({}, {})", num_columns, num_rows);
  if (num_columns > 0) {
    if (A_.cols() < num_columns) A_.conservativeResize(Eigen::NoChange, num_columns);
    if (c_.size() < num_columns) c_.conservativeResize(num_columns);
    if (x_lb_.size() < num_columns) x_lb_.conservativeResize(num_columns);
    x_ub_.reserve(num_columns);
  }
  if (num_rows > 0) {
    if (A_.rows() < num_rows) A_.conservativeResize(num_rows, Eigen::NoChange);
    if (b_.size() < num_rows) b_.conservativeResize(num_rows);
  }
}
void LpProblem::SlackForm(Matrix<T>& slack_A, Vector<T>& slack_b, Vector<T>& slack_c) const {
  DELPI_TRACE_FMT("LpProblem::SlackForm({}, {}, {})", slack_A, slack_b, slack_c);

  // TODO(tend): if we make the method not const we can avoid this copy and work directly on x_lb_, x_ub_ and A_
  Vector<T> x_lb{x_lb_};
  std::unordered_map<Index, mpq_class> x_ub{x_ub_};
  for (Index i = 0; i < num_columns_; ++i) {
    if (gmp::IsInfinity(x_lb_.coeff(i))) {
      // TODO(tend): handle free variables
      DELPI_ASSERT(x_ub_.contains(i), "Upper bound must be finite");
      x_lb(i) = -x_ub.at(i);
      x_ub.erase(i);
      slack_A.col(i) *= -1;
    } else if (x_ub.contains(i)) {
      x_ub.at(i) = x_ub.at(i) - x_lb(i);
    }
  }
  const Index num_ub = static_cast<Index>(x_ub.size());
  // Now we have the upper bounds on the original variables in x_ub and the lower bounds in x_lb
  // All the bounds will be added as new rows in the slack_A matrix

  slack_A = Matrix<T>(num_rows_ + num_ub, num_columns_ + num_rows_ + num_ub);
  slack_A.block(0, 0, A_.rows(), A_.cols()) = A_;
  slack_A.rightCols(num_rows_ + num_ub).setIdentity();
  Index num_row = num_rows_;
  for (const auto& [idx, value] : x_ub) {
    slack_A(num_row++, idx) = 1;
  }
  for (Index i = 0; i < num_rows_; ++i) {
    switch (sense_[i]) {
      case FormulaKind::Eq:
        slack_A(i, num_columns_ + i) = 0;
        break;
      case FormulaKind::Leq:
        break;
      case FormulaKind::Geq:
        slack_A(i, num_columns_ + i) = -1;
        break;
      default:
        DELPI_UNREACHABLE();
    }
  }

  // Set the rhs
  slack_b = Vector<T>{num_rows_ + num_ub};
  slack_b.head(num_rows_) = b_;
  num_row = num_rows_;
  for (const auto& [idx, value] : x_ub) {
    slack_b(num_row++) = value;
  }
  x_lb.conservativeResize(num_columns_ + num_rows_ + num_ub);
  x_lb.tail(num_rows_ + num_ub) = Vector<T>::Zero(num_rows_ + num_ub);
  slack_b = (slack_b - slack_A * x_lb).eval();

  slack_c = Vector<T>::Zero(num_columns_ + num_rows_ + num_ub);
  slack_c.head(num_columns_) = c_;
}

void LpProblem::FixSolution(Vector<T>& x) {
  DELPI_TRACE_FMT("LpProblem::FixSolution({})", x);
  x.conservativeResize(num_columns_);
  for (Index i = 0; i < num_columns_; ++i) {
    if (gmp::IsInfinity(x_lb_.coeff(i))) {
      DELPI_ASSERT(x_ub_.contains(i), "Upper bound must be finite");
      x(i) -= x_ub_.at(i);
      x(i) = -x(i);
    } else {
      x(i) += x_lb_.coeff(i);
    }
  }
}
Vector<mpq_class> LpProblem::rhs() const {
  DELPI_TRACE("LpProblem::rhs()");
  Vector<T> rhs{num_rows_ + static_cast<Index>(x_ub_.size())};
  rhs.head(num_rows_) = b_;
  Index num_row = num_rows_;
  for (const auto& [idx, value] : x_ub_) {
    rhs(num_row++) = value;
  }
  return rhs;
}

std::ostream& operator<<(std::ostream& os, const LpProblem& problem) {
  os << "Minimise:\n" << problem.c().transpose() << "\nSubject to:\n";
  for (const Row& x : problem.rows()) {
    os << x << "\n";
  }
  return os;
}

}  // namespace delpi::internal
