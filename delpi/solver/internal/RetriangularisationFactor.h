#pragma once

#include <iosfwd>
#include <vector>

#include "delpi/libs/eigen.h"

namespace delpi::internal {

template <class T>
class RetriangularisationFactor {
 public:
  RetriangularisationFactor(T value, int row, bool permuted = false, bool inverted = false);

  [[nodiscard]] RetriangularisationFactor Permutation() const { return {value_, row_, !permuted_, inverted_}; }
  RetriangularisationFactor& Permute() {
    permuted_ = !permuted_;
    return *this;
  }
  [[nodiscard]] RetriangularisationFactor Inverse() const { return {-value_, row_, permuted_, !inverted_}; }
  RetriangularisationFactor& Invert() {
    value_ = -value_;
    inverted_ = !inverted_;
    return *this;
  }

  [[nodiscard]] T value() const { return value_; }
  [[nodiscard]] int row() const { return row_; }
  [[nodiscard]] bool permuted() const { return permuted_; }
  [[nodiscard]] bool inverted() const { return inverted_; }

 private:
  T value_;
  int row_;
  bool permuted_;
  bool inverted_;
};

template <class T>
Matrix<T>& operator*=(Matrix<T>& matrix, const RetriangularisationFactor<T>& factor);
template <class T>
Matrix<T> operator*(const RetriangularisationFactor<T>& factor, const Matrix<T>& matrix);

template <class T>
Matrix<T>& operator*=(const RetriangularisationFactor<T>& factor, Matrix<T>& matrix);
template <class T>
Matrix<T> operator*(const Matrix<T>& matrix, const RetriangularisationFactor<T>& factor);

template <class T>
Matrix<T>& operator*=(Matrix<T>& matrix, const std::vector<RetriangularisationFactor<T>>& factors);
template <class T>
Matrix<T>& operator*=(const std::vector<RetriangularisationFactor<T>>& factors, Matrix<T>& matrix);

template <class T>
Matrix<T> operator*(const Matrix<T>& matrix, const std::vector<RetriangularisationFactor<T>>& factors);
template <class T>
Matrix<T> operator*(const std::vector<RetriangularisationFactor<T>>& factors, const Matrix<T>& matrix);

template <class T>
std::ostream& operator<<(std::ostream& os, const RetriangularisationFactor<T>& factor);

}  // namespace delpi::internal

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::internal::RetriangularisationFactor<mpq_class>)
OSTREAM_FORMATTER(delpi::internal::RetriangularisationFactor<double>)

#endif
