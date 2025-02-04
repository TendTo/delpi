/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * BgLinearSystemSolver class.
 */
#pragma once

#include <iosfwd>
#include <vector>

#include "LinearSystemSolver.h"
#include "RetriangularisationFactor.h"

namespace delpi::internal {

template <class T>
class BgLinearSystemSolver final : public LinearSystemSolver<T> {
 public:
  using LowerMatrix = Eigen::TriangularView<const Matrix<T>, Eigen::UnitLower>;
  using UpperMatrix = Eigen::TriangularView<const Matrix<T>, Eigen::Upper>;
  using PermutationMatrix = Eigen::PermutationMatrix<Eigen::Dynamic>;

  explicit BgLinearSystemSolver(const Config& config);

  [[nodiscard]] LowerMatrix L() const { return L_.template triangularView<Eigen::UnitLower>(); }
  [[nodiscard]] UpperMatrix U() const { return U_.template triangularView<Eigen::Upper>(); }
  // [[nodiscard]] const std::vector<RetriangularisationFactor<T>>& factors() const { return factors_; }
  [[nodiscard]] const PermutationMatrix& P() const { return P_; }
  [[nodiscard]] Matrix<T> B() const;

 private:
  Vector<T> SolveCore(const Vector<T>& vector) const override;
  Vector<T> TransposeSolveCore(const Vector<T>& vector) const override;
  void FactoriseCore(const Basis<T>& basis) override;
  void ResetCore() override;

  void FactoriseFromScratch(const Basis<T>& basis);
  void UpdateFactorisation(const Basis<T>& basis);

  size_t n_iteration_;
  Matrix<T> L_;
  // std::vector<RetriangularisationFactor<T>> factors_;
  std::vector<Matrix<T>> factors_;
  Matrix<T> U_;
  PermutationMatrix P_;
};

template <class T>
std::ostream& operator<<(std::ostream& os, const BgLinearSystemSolver<T>& solver);

}  // namespace delpi::internal
