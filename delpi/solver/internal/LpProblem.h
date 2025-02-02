#pragma once

#include <iosfwd>
#include <unordered_map>
#include <vector>

#include "delpi/libs/eigen.h"
#include "delpi/libs/gmp.h"
#include "delpi/solver/internal/Column.h"
#include "delpi/solver/internal/Row.h"
#include "delpi/symbolic/FormulaKind.h"

namespace delpi::internal {

using T = mpq_class;
class LpProblem {
 public:
  LpProblem() : num_columns_{0}, num_rows_{0} {}

  [[nodiscard]] const SMatrix<T>& A() const { return A_; }
  [[nodiscard]] const Vector<T>& c() const { return c_; }
  [[nodiscard]] const SVector<T>& l() const { return x_lb_; }
  [[nodiscard]] Vector<mpq_class> rhs() const;
  [[nodiscard]] const Vector<T>& b() const { return b_; }
  [[nodiscard]] const std::vector<Index>& free_vars() const { return free_vars_; }
  [[nodiscard]] Index num_columns() const { return num_columns_; }
  [[nodiscard]] Index num_rows() const { return num_rows_; }

  [[nodiscard]] Row row(Index row_idx) const;
  [[nodiscard]] Column column(Index column_idx) const;
  [[nodiscard]] std::vector<Row> rows() const;
  [[nodiscard]] std::vector<Column> columns() const;

  void AddColumn(const T& obj, const T& lb, const T& ub);
  void AddRow(const std::unordered_map<Index, T>& row, const T& lb, const T& ub);
  void SetObjective(Index column_idx, const T& value);
  void Reserve(Index num_rows, Index num_columns);

  void SlackForm(Matrix<T>& slack_A, Vector<T>& slack_b, Vector<T>& slack_c) const;

  void FixSolution(Vector<T>& x);

 private:
  Index num_columns_;
  Index num_rows_;

  SMatrix<T> A_;
  Vector<T> c_;
  Vector<T> b_;
  std::vector<FormulaKind> sense_;

  SVector<T> x_lb_;
  std::unordered_map<Index, T> x_ub_;
  std::vector<Index> free_vars_;
};

std::ostream& operator<<(std::ostream& os, const LpProblem& problem);

}  // namespace delpi::internal

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::internal::LpProblem)

#endif
