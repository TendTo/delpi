/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/solver/internal/RetriangularisationFactor.h"

#include <ostream>

#include "delpi/util/error.h"

namespace delpi::internal {

template <class T>
RetriangularisationFactor<T>::RetriangularisationFactor(T value, const int row, const bool permuted,
                                                        const bool inverted)
    : value_{std::move(value)}, row_{row}, permuted_{permuted}, inverted_{inverted} {
  DELPI_ASSERT(row >= 0, "Row index cannot be negative");
}
template <class T>
Matrix<T>& operator*=(Matrix<T>& matrix, const RetriangularisationFactor<T>& factor) {
  DELPI_ASSERT(factor.row() < matrix.rows() - 1, "Invalid row index");
  const int pivot_row = factor.row();

  // Swap the columns if the factor is permuted and inverted
  if (factor.permuted() && factor.inverted()) matrix.col(pivot_row).swap(matrix.col(pivot_row + 1));

  matrix.col(pivot_row) += matrix.col(pivot_row + 1) * factor.value();

  // Swap the columns if the factor is permuted and not inverted
  if (factor.permuted() && !factor.inverted()) matrix.col(pivot_row).swap(matrix.col(pivot_row + 1));

  return matrix;
}
template <class T>
Matrix<T>& operator*=(const RetriangularisationFactor<T>& factor, Matrix<T>& matrix) {
  DELPI_ASSERT(factor.row() < matrix.rows() - 1, "Invalid row index");
  const int pivot_row = factor.row();
  // Swap the rows if the factor is permuted and not inverted
  if (factor.permuted() && !factor.inverted()) matrix.row(pivot_row).swap(matrix.row(pivot_row + 1));

  // Update the pivot row
  matrix.row(pivot_row + 1) += matrix.row(pivot_row) * factor.value();

  // Swap the rows if the factor is permuted and inverted
  if (factor.permuted() && factor.inverted()) matrix.row(pivot_row).swap(matrix.row(pivot_row + 1));

  return matrix;
}

template <class T>
Matrix<T> operator*(const Matrix<T>& matrix, const RetriangularisationFactor<T>& factor) {
  Matrix<T> result{matrix};
  return result *= factor;
}
template <class T>
Matrix<T> operator*(const RetriangularisationFactor<T>& factor, const Matrix<T>& matrix) {
  Matrix<T> result{matrix};
  return factor *= result;
}
template <class T>
Eigen::MatrixX<T>& operator*=(Eigen::MatrixX<T>& matrix, const std::vector<RetriangularisationFactor<T>>& factors) {
  for (const RetriangularisationFactor<T>& factor : factors) matrix *= factor.Inverse();
  return matrix;
}
template <class T>
Matrix<T>& operator*=(const std::vector<RetriangularisationFactor<T>>& factors, Matrix<T>& matrix) {
  for (const RetriangularisationFactor<T>& factor : factors) factor *= matrix;
  return matrix;
}
template <class T>
Matrix<T> operator*(const Matrix<T>& matrix, const std::vector<RetriangularisationFactor<T>>& factors) {
  Matrix<T> result{matrix};
  return result *= factors;
}
template <class T>
Matrix<T> operator*(const std::vector<RetriangularisationFactor<T>>& factors, const Matrix<T>& matrix) {
  Matrix<T> result{matrix};
  return factors *= result;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const RetriangularisationFactor<T>& factor) {
  Matrix<T> matrix = Matrix<T>::Identity(factor.row() + 2, factor.row() + 2);
  if (factor.permuted()) {
    matrix(factor.row() + 1, factor.row() + 1) = factor.value();
    matrix(factor.row(), factor.row() + 1) = static_cast<T>(1);
  } else {
    matrix(factor.row() + 1, factor.row()) = factor.value();
  }
  return os << matrix;
}

template class RetriangularisationFactor<mpq_class>;
template class RetriangularisationFactor<double>;

template Matrix<mpq_class>& operator*=(Matrix<mpq_class>&, const RetriangularisationFactor<mpq_class>&);
template Matrix<double>& operator*=(Matrix<double>&, const RetriangularisationFactor<double>&);
template Matrix<mpq_class>& operator*=(const RetriangularisationFactor<mpq_class>&, Matrix<mpq_class>&);
template Matrix<double>& operator*=(const RetriangularisationFactor<double>&, Matrix<double>&);

template Matrix<mpq_class> operator*(const Matrix<mpq_class>&, const RetriangularisationFactor<mpq_class>&);
template Matrix<double> operator*(const Matrix<double>&, const RetriangularisationFactor<double>&);
template Matrix<mpq_class> operator*(const RetriangularisationFactor<mpq_class>&, const Matrix<mpq_class>&);
template Matrix<double> operator*(const RetriangularisationFactor<double>&, const Matrix<double>&);

template Matrix<mpq_class>& operator*=(Matrix<mpq_class>&, const std::vector<RetriangularisationFactor<mpq_class>>&);
template Matrix<double>& operator*=(Matrix<double>&, const std::vector<RetriangularisationFactor<double>>&);
template Matrix<mpq_class>& operator*=(const std::vector<RetriangularisationFactor<mpq_class>>&, Matrix<mpq_class>&);
template Matrix<double>& operator*=(const std::vector<RetriangularisationFactor<double>>&, Matrix<double>&);

template Matrix<mpq_class> operator*(const Matrix<mpq_class>&,
                                     const std::vector<RetriangularisationFactor<mpq_class>>&);
template Matrix<double> operator*(const Matrix<double>&, const std::vector<RetriangularisationFactor<double>>&);
template Matrix<mpq_class> operator*(const std::vector<RetriangularisationFactor<mpq_class>>&,
                                     const Matrix<mpq_class>&);
template Matrix<double> operator*(const std::vector<RetriangularisationFactor<double>>&, const Matrix<double>&);

template std::ostream& operator<<(std::ostream&, const RetriangularisationFactor<mpq_class>&);
template std::ostream& operator<<(std::ostream&, const RetriangularisationFactor<double>&);

}  // namespace delpi::internal
