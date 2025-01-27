/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/solver/internal/Basis.h"
#include "delpi/util/Config.h"

using delpi::Config;
using delpi::internal::Basis;

template <class T>
class TestBasis : public ::testing::Test {
 protected:
  Eigen::MatrixX<T> matrix_ = Eigen::MatrixX<T>::Ones(10, 20);
  Config config_;
  TestBasis() : config_{Config{}} {
    for (int i = 0; i < matrix_.cols(); i++) matrix_.col(i) *= i;
  }
};

using types = ::testing::Types<double, int, mpq_class>;
TYPED_TEST_SUITE(TestBasis, types);

TYPED_TEST(TestBasis, Constructor) {
  const std::vector basis_idxs{1, 2, 4, 19};
  const Basis<TypeParam> basis(this->matrix_, basis_idxs);

  ASSERT_EQ(basis_idxs.size(), basis.basis_idxs().size());
  ASSERT_EQ(basis_idxs.size(), basis.basis_vectors().cols());
  for (int i = 0; i < static_cast<int>(basis_idxs.size()); i++) {
    EXPECT_EQ(basis.basis_idxs()[i], basis_idxs[i]);
    EXPECT_EQ(basis.basis_vectors().col(i), this->matrix_.col(basis_idxs[i]));
    EXPECT_EQ(basis.basis_vectors()(0, i), static_cast<TypeParam>(basis.basis_idxs()[i]));
  }
}

TYPED_TEST(TestBasis, SharedPointerConstructor) {
  const std::shared_ptr basis_idxs{std::make_shared<std::vector<int>>(std::vector{1, 2, 4, 19})};
  const Basis<TypeParam> basis(this->matrix_, basis_idxs);

  EXPECT_EQ(basis_idxs.get(), &basis.basis_idxs());
  ASSERT_EQ(basis_idxs->size(), basis.basis_idxs().size());
  ASSERT_EQ(basis_idxs->size(), basis.basis_vectors().cols());
  for (int i = 0; i < static_cast<int>(basis_idxs->size()); i++) {
    EXPECT_EQ(basis.basis_idxs()[i], basis_idxs->at(i));
    EXPECT_EQ(basis.basis_vectors().col(i), this->matrix_.col(basis_idxs->at(i)));
    EXPECT_EQ(basis.basis_vectors()(0, i), static_cast<TypeParam>(basis.basis_idxs()[i]));
  }
}

TYPED_TEST(TestBasis, CopyConstructor) {
  Eigen::MatrixXf float_matrix = Eigen::MatrixXf::Ones(10, 20);
  const std::shared_ptr basis_idxs{std::make_shared<std::vector<int>>(std::vector{1, 2, 4, 19})};
  const Basis<TypeParam> float_basis(this->matrix_, basis_idxs);

  const Basis<TypeParam> basis(this->matrix_, float_basis);

  EXPECT_EQ(basis_idxs.get(), &basis.basis_idxs());
  ASSERT_EQ(basis_idxs->size(), basis.basis_idxs().size());
  ASSERT_EQ(basis_idxs->size(), basis.basis_vectors().cols());
  for (int i = 0; i < static_cast<int>(basis_idxs->size()); i++) {
    EXPECT_EQ(basis.basis_idxs()[i], basis_idxs->at(i));
    EXPECT_EQ(basis.basis_vectors().col(i), this->matrix_.col(basis_idxs->at(i)));
    EXPECT_EQ(basis.basis_vectors()(0, i), static_cast<TypeParam>(basis.basis_idxs()[i]));
  }
}
