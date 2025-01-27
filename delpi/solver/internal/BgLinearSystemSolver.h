#pragma once

#include <iosfwd>

#include "LinearSystemSolver.h"

namespace delpi::internal {

template <class T>
// using T = double;
// template <class B>
class BgLinearSystemSolver final : public LinearSystemSolver<T> {
 public:
  using typename LinearSystemSolver<T>::Vector;
  using typename LinearSystemSolver<T>::Matrix;
  using LowerMatrix = Eigen::TriangularView<const Matrix, Eigen::UnitLower>;
  using UpperMatrix = Eigen::TriangularView<const Matrix, Eigen::Upper>;
  using PermutationMatrix = Eigen::PermutationMatrix<Eigen::Dynamic>;

  explicit BgLinearSystemSolver(const Config& config);

  LowerMatrix L() const { return LU_.template triangularView<Eigen::UnitLower>(); }
  UpperMatrix U() const { return LU_.template triangularView<Eigen::Upper>(); }
  const PermutationMatrix& P() const { return P_; }

 private:
  Vector SolveCore(const Vector& vector) const override;
  Vector TransposeSolveCore(const Vector& vector) const override;
  void FactoriseCore(const Basis<T>& basis) override;
  void ResetCore() override;

  void FactoriseFromScratch(const Basis<T>& basis);
  void UpdateFactorisation(const Basis<T>& basis);

  size_t n_iteration_;
  Matrix LU_;
  PermutationMatrix P_;
};

template <class T>
std::ostream& operator<<(std::ostream& os, const BgLinearSystemSolver<T>& solver);

}  // namespace delpi::internal
