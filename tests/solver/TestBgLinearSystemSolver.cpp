/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/solver/internal/BgLinearSystemSolver.h"
#include "delpi/util/Config.h"

using delpi::Config;
using delpi::Matrix;
using delpi::internal::Basis;
using delpi::internal::BgLinearSystemSolver;

namespace {
template <class T>
Matrix<T> ToEigenMatrix(const std::vector<std::vector<T>>& matrix) {
  Matrix<T> result(matrix.size(), matrix[0].size());
  for (int i = 0; i < static_cast<int>(matrix.size()); ++i) {
    for (int j = 0; j < static_cast<int>(matrix[0].size()); ++j) {
      result(i, j) = matrix[i][j];
    }
  }
  return result;
}
}  // namespace

template <class T>
class TestBgLinearSystemSolver : public ::testing::Test {
 protected:
  Matrix<T> matrix_{4, 5};

  const std::vector<int> basis_idxs_{0, 1, 2, 3};
  Config config_;
  TestBgLinearSystemSolver() : config_{Config{}} {
    matrix_ << 1, 2, 3, 4, 21, 5, 6, 7, 8, 22, 9, 10, 15, 17, 23, 13, 14, 15, 19, 24;
  }
};

using types = ::testing::Types<double, mpq_class>;
TYPED_TEST_SUITE(TestBgLinearSystemSolver, types);

TYPED_TEST(TestBgLinearSystemSolver, Factorise) {
  BgLinearSystemSolver<TypeParam> solver{this->config_};
  Basis<TypeParam> basis{this->matrix_, this->basis_idxs_};
  solver.Factorise(basis);
  EXPECT_EQ(solver.L().toDenseMatrix() * solver.U().toDenseMatrix(), solver.P() * basis.basis_vectors());
}

TYPED_TEST(TestBgLinearSystemSolver, UpdateFactorisation) {
  BgLinearSystemSolver<TypeParam> solver{this->config_};
  Basis<TypeParam> basis{this->matrix_, this->basis_idxs_};
  solver.Factorise(basis);
  EXPECT_EQ(solver.L().toDenseMatrix() * solver.U().toDenseMatrix(), solver.P() * basis.basis_vectors());
  basis.Update(this->matrix_, 2, 4);
  std::cout << basis.basis_vectors() << std::endl;
  solver.Factorise(basis);
}
