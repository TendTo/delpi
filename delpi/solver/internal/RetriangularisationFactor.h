#pragma once

#include <algorithm>
#include <iosfwd>

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
Eigen::MatrixX<T>& operator*=(Eigen::MatrixX<T>& matrix, const RetriangularisationFactor<T>& factor);
template <class T>
Eigen::MatrixX<T> operator*(const RetriangularisationFactor<T>& factor, const Eigen::MatrixX<T>& matrix);

template <class T>
Eigen::MatrixX<T>& operator*=(const RetriangularisationFactor<T>& factor, Eigen::MatrixX<T>& matrix);
template <class T>
Eigen::MatrixX<T> operator*(const Eigen::MatrixX<T>& matrix, const RetriangularisationFactor<T>& factor);

template <class T>
std::ostream& operator<<(std::ostream& os, const RetriangularisationFactor<T>& factor);

}  // namespace delpi::internal
