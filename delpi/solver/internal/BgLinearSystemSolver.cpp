
#include "BgLinearSystemSolver.h"

#include <iostream>
#include <ostream>

#include "delpi/util/error.h"

namespace delpi::internal {

template <class T>
BgLinearSystemSolver<T>::BgLinearSystemSolver(const Config& config)
    : LinearSystemSolver<T>(config, "BGLinearSystemSolver"), n_iteration_{0}, LU_{} {}
template <class T>
typename LinearSystemSolver<T>::Vector BgLinearSystemSolver<T>::SolveCore(const Vector&) const {
  DELPI_UNREACHABLE();
}
template <class T>
typename LinearSystemSolver<T>::Vector BgLinearSystemSolver<T>::TransposeSolveCore(const Vector&) const {
  DELPI_UNREACHABLE();
}
template <class T>
void BgLinearSystemSolver<T>::FactoriseCore(const Basis<T>& basis) {
  if (n_iteration_ == 0)
    FactoriseFromScratch(basis);
  else
    UpdateFactorisation(basis);

  n_iteration_++;
  // TODO(tend): use proper max_iteration_before_refactor config
  // if (n_iteration_ >= LinearSystemSolver<T>::config_.max_iteration_before_refactor()) n_iteration_ = 0;
  if (n_iteration_ >= 10) n_iteration_ = 0;
}
template <class T>
void BgLinearSystemSolver<T>::ResetCore() {
  DELPI_UNREACHABLE();
}
template <class T>
void BgLinearSystemSolver<T>::FactoriseFromScratch(const Basis<T>& basis) {
  DELPI_DEBUG_FMT("FactoriseFromScratch({})", basis);
  // Run a simple LU decomposition
  const Eigen::PartialPivLU<Matrix> lu_decomposition = basis.basis_vectors().partialPivLu();
  LU_ = lu_decomposition.matrixLU();
  P_ = lu_decomposition.permutationP();
}
template <class T>
void BgLinearSystemSolver<T>::UpdateFactorisation(const Basis<T>& basis) {
  DELPI_ASSERT(LU_.cols() > 0, "LU must be initialized");
  DELPI_ASSERT(basis.last_basis_leaving() < basis.last_basis_entering(), "last_out must be less than last_in");
  DELPI_TRACE_FMT("UpdateFactorisation({})", basis);
  // Update the LU decomposition using the Bartels-Golub method
  // B = [B1, B2, ... Bl-1, Bl+1, ... Be]       where Bl is the last out and Be is the last in
  // H = [U1, U2, ... Ul-1, Ul+1, ... L^-1Be]   we create a hessemberg matrix H
  Matrix H = LU_.template triangularView<Eigen::Upper>().toDenseMatrix();
  for (int i = basis.last_basis_leaving(); i < basis.last_basis_entering(); ++i) {
    H.col(i).swap(H.col(i + 1));
  }
  H.col(basis.last_basis_entering()) = LU_.template triangularView<Eigen::UnitLower>().solve(
      P_ * basis.basis_vectors().col(basis.last_basis_entering()));

  // std::cout << H << std::endl;
  // // Lastly, we need to factorise the H matrix to get an upper triangular matrix
  // for (int i = basis.last_basis_leaving(); i < basis.last_basis_entering(); ++i) {
  //   const T factor = H(i + 1, i) / H(i, i);
  //   H(i + 1, Eigen::seq(i + 1, H.cols() - 1)) -= H(i, Eigen::seq(i + 1, H.cols() - 1)) * factor;
  // }
  // std::cout << H << std::endl;
  // // Update the LU decomposition
  auto L = LU_.template triangularView<Eigen::UnitLower>();
  std::cout << (L * H) << std::endl;
  LU_.template triangularView<Eigen::Upper>() = H.template triangularView<Eigen::Upper>();
}

template <class T>
std::ostream& operator<<(std::ostream& os, const BgLinearSystemSolver<T>& solver) {
  return os << "BGLinearSystemSolver\nL\n"
            << solver.L().toDenseMatrix() << "\nU\n"
            << solver.U().toDenseMatrix() << "\n";
}

template class BgLinearSystemSolver<mpq_class>;
template class BgLinearSystemSolver<double>;

template std::ostream& operator<<(std::ostream& os, const BgLinearSystemSolver<mpq_class>& solver);
template std::ostream& operator<<(std::ostream& os, const BgLinearSystemSolver<double>& solver);

}  // namespace delpi::internal
