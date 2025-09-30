/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/solver/internal/RetriangularisationFactor.h"
#include "delpi/util/exception.h"

using delpi::DelpiAssertionException;
using delpi::Matrix;
using delpi::Vector;
using delpi::internal::RetriangularisationFactor;

template <class T>
class TestRetriangularisationFactor : public ::testing::Test {
 protected:
  Matrix<T> matrix_{4, 4};
  Matrix<T> L_;
  Matrix<T> U_;
  const T value_ = 2;
  TestRetriangularisationFactor() {
    matrix_ << 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15;
    L_ = matrix_.template triangularView<Eigen::Lower>().toDenseMatrix();
    U_ = matrix_.template triangularView<Eigen::Upper>().toDenseMatrix();
    U_.diagonal(-1) = Vector<T>::Ones(3);
  }

  Matrix<T> GetMatrixFactor(const int pivot_row, const bool permute = false) {
    Eigen::MatrixX<T> matrix_factor = Eigen::MatrixX<T>::Identity(L_.rows(), L_.cols());
    matrix_factor(pivot_row + 1, pivot_row) = this->value_;
    Eigen::PermutationMatrix<Eigen::Dynamic> P(L_.rows());
    P.setIdentity();
    if (permute) P.applyTranspositionOnTheRight(pivot_row, pivot_row + 1);
    return matrix_factor * P;
  }
};

using Types = ::testing::Types<double, mpq_class>;
TYPED_TEST_SUITE(TestRetriangularisationFactor, Types);

TYPED_TEST(TestRetriangularisationFactor, LowerTriangularTimesFactor) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->L_ * this->GetMatrixFactor(pivot_row), this->L_ * factor);
  }
}

TYPED_TEST(TestRetriangularisationFactor, LowerTriangularTimesFactorInverse) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->L_ * this->GetMatrixFactor(pivot_row).inverse(), this->L_ * factor.Inverse());
  }
}

TYPED_TEST(TestRetriangularisationFactor, LowerTriangularTimesFactorPermuted) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->L_ * this->GetMatrixFactor(pivot_row, true), this->L_ * factor.Permutation());
  }
}

TYPED_TEST(TestRetriangularisationFactor, LowerTriangularTimesFactorPermutedInverse) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->L_ * this->GetMatrixFactor(pivot_row, true).inverse(), this->L_ * factor.Permutation().Inverse());
  }
}

#ifndef NDEBUG
TYPED_TEST(TestRetriangularisationFactor, LowerTriangularTimesFactorInvalidRow) {
  for (int i = 0; i < this->L_.cols() + 1; ++i) {
    RetriangularisationFactor<TypeParam> factor{0, i};
    if (i >= this->L_.cols() - 1) {
      EXPECT_THROW(this->L_ * factor, DelpiAssertionException);
    } else {
      EXPECT_NO_THROW(this->L_ * factor);
    }
  }
}
#endif

TYPED_TEST(TestRetriangularisationFactor, FactorTimesUpperTriangular) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->GetMatrixFactor(pivot_row) * this->U_, factor * this->U_);
  }
}

TYPED_TEST(TestRetriangularisationFactor, FactorTimesUpperTriangularInverse) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->GetMatrixFactor(pivot_row).inverse() * this->U_, factor.Inverse() * this->U_);
  }
}

TYPED_TEST(TestRetriangularisationFactor, FactorTimesUpperTriangularPermuted) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->GetMatrixFactor(pivot_row, true) * this->U_, factor.Permutation() * this->U_);
  }
}

TYPED_TEST(TestRetriangularisationFactor, FactorTimesUpperTriangularPermutedInverse) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->GetMatrixFactor(pivot_row, true).inverse() * this->U_, factor.Permutation().Inverse() * this->U_);
  }
}

#ifndef NDEBUG
TYPED_TEST(TestRetriangularisationFactor, FactorTimesUpperTriangularInvalidRow) {
  for (int i = 0; i < this->U_.cols() + 1; ++i) {
    RetriangularisationFactor<TypeParam> factor{0, i};
    if (i >= this->U_.cols() - 1) {
      EXPECT_THROW(factor * this->U_, DelpiAssertionException);
    } else {
      EXPECT_NO_THROW(factor * this->U_);
    }
  }
}
#endif

TYPED_TEST(TestRetriangularisationFactor, FactorTimesMatrix) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->GetMatrixFactor(pivot_row) * this->matrix_, factor * this->matrix_);
  }
}
TYPED_TEST(TestRetriangularisationFactor, FactorTimesMatrixInverse) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->GetMatrixFactor(pivot_row).inverse() * this->matrix_, factor.Inverse() * this->matrix_);
  }
}
TYPED_TEST(TestRetriangularisationFactor, FactorTimesMatrixPermuted) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->GetMatrixFactor(pivot_row, true) * this->matrix_, factor.Permutation() * this->matrix_);
  }
}
TYPED_TEST(TestRetriangularisationFactor, FactorTimesMatrixPermutedInverse) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->GetMatrixFactor(pivot_row, true).inverse() * this->matrix_,
              factor.Permutation().Inverse() * this->matrix_);
  }
}
#ifndef NDEBUG
TYPED_TEST(TestRetriangularisationFactor, FactorTimesMatrixInvalidRow) {
  for (int i = 0; i < this->matrix_.cols() + 1; ++i) {
    RetriangularisationFactor<TypeParam> factor{0, i};
    if (i >= this->matrix_.cols() - 1) {
      EXPECT_THROW(factor * this->matrix_, DelpiAssertionException);
    } else {
      EXPECT_NO_THROW(factor * this->matrix_);
    }
  }
}
#endif

TYPED_TEST(TestRetriangularisationFactor, MatrixTimesFactor) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->matrix_ * this->GetMatrixFactor(pivot_row), this->matrix_ * factor);
  }
}
TYPED_TEST(TestRetriangularisationFactor, MatrixTimesFactorInverse) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->matrix_ * this->GetMatrixFactor(pivot_row).inverse(), this->matrix_ * factor.Inverse());
  }
}
TYPED_TEST(TestRetriangularisationFactor, MatrixTimesFactorPermuted) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->matrix_ * this->GetMatrixFactor(pivot_row, true), this->matrix_ * factor.Permutation());
  }
}
TYPED_TEST(TestRetriangularisationFactor, MatrixTimesFactorPermutedInverse) {
  for (int pivot_row = 0; pivot_row < this->L_.cols() - 1; ++pivot_row) {
    RetriangularisationFactor<TypeParam> factor{this->value_, pivot_row};
    EXPECT_EQ(this->matrix_ * this->GetMatrixFactor(pivot_row, true).inverse(),
              this->matrix_ * factor.Permutation().Inverse());
  }
}
#ifndef NDEBUG
TYPED_TEST(TestRetriangularisationFactor, MatrixTimesFactorInvalidRow) {
  for (int i = 0; i < this->matrix_.cols() + 1; ++i) {
    RetriangularisationFactor<TypeParam> factor{0, i};
    if (i >= this->matrix_.cols() - 1) {
      EXPECT_THROW(this->matrix_ * factor, DelpiAssertionException);
    } else {
      EXPECT_NO_THROW(this->matrix_ * factor);
    }
  }
}
#endif
