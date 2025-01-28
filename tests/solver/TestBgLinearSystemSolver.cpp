/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include <gtest/gtest.h>

#include "delpi/solver/internal/BgLinearSystemSolver.h"
#include "delpi/util/Config.h"
#include "delpi/util/error.h"

using delpi::Config;
using delpi::Matrix;
using delpi::Vector;
using delpi::internal::Basis;
using delpi::internal::BgLinearSystemSolver;

template <class T>
class TestBgLinearSystemSolver : public ::testing::Test {
 protected:
  Matrix<T> matrix_{3, 6};
  Vector<T> vector_{3};

  const std::vector<int> basis_idxs_{0, 1, 2};
  Config config_;
  TestBgLinearSystemSolver() : config_{Config{}} {
    matrix_ << 1, 2, 3, 4, 5, 6,  //
        13, 14, 15, 16, 17, 18,   //
        12, 11, 9, 8, 7, 6;       //
    vector_ << 31, 17, 13;
  }
};

using Types = ::testing::Types<mpq_class>;
TYPED_TEST_SUITE(TestBgLinearSystemSolver, Types);

TYPED_TEST(TestBgLinearSystemSolver, Factorise) {
  BgLinearSystemSolver<TypeParam> solver{this->config_};
  Basis<TypeParam> basis{this->matrix_, this->basis_idxs_};
  solver.Factorise(basis);
  EXPECT_EQ(solver.L().toDenseMatrix() * solver.U().toDenseMatrix(), solver.P() * basis.basis_vectors());
  EXPECT_EQ(solver.B(), basis.basis_vectors());
  EXPECT_EQ(solver.Solve(this->vector_), basis.basis_vectors().fullPivLu().solve(this->vector_));
}

TYPED_TEST(TestBgLinearSystemSolver, UpdateFactorisationNoFactorsNeeded) {
  BgLinearSystemSolver<TypeParam> solver{this->config_};
  Basis<TypeParam> basis{this->matrix_, this->basis_idxs_};
  solver.Factorise(basis);
  basis.Update(this->matrix_, this->matrix_.rows() - 1, this->matrix_.rows());
  solver.Factorise(basis);
  EXPECT_EQ(solver.B(), basis.basis_vectors());
  EXPECT_EQ(solver.Solve(this->vector_), basis.basis_vectors().fullPivLu().solve(this->vector_));
}

TYPED_TEST(TestBgLinearSystemSolver, UpdateFactorisationFactors) {
  BgLinearSystemSolver<TypeParam> solver{this->config_};
  Basis<TypeParam> basis{this->matrix_, this->basis_idxs_};
  solver.Factorise(basis);
  basis.Update(this->matrix_, 0, this->matrix_.rows());
  solver.Factorise(basis);
  EXPECT_EQ(solver.B(), basis.basis_vectors());
  EXPECT_EQ(solver.Solve(this->vector_), basis.basis_vectors().fullPivLu().solve(this->vector_));
}

TYPED_TEST(TestBgLinearSystemSolver, MultipleUpdateFactorisation) {
  BgLinearSystemSolver<TypeParam> solver{this->config_};
  Basis<TypeParam> basis{this->matrix_, this->basis_idxs_};
  solver.Factorise(basis);
  for (int i = 0; i < this->matrix_.rows(); ++i) {
    basis.Update(this->matrix_, i, this->matrix_.rows() + i);
    solver.Factorise(basis);
    EXPECT_EQ(solver.B(), basis.basis_vectors());
    EXPECT_EQ(solver.Solve(this->vector_), basis.basis_vectors().fullPivLu().solve(this->vector_));
  }
}
