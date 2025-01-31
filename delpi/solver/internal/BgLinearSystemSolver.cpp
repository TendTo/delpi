
#include "delpi/solver/internal/BgLinearSystemSolver.h"

#include <ostream>
#include <ostream>
#include <ranges>

#include "delpi/util/error.h"
#include "delpi/util/logging.h"

namespace delpi::internal {

template <class T>
BgLinearSystemSolver<T>::BgLinearSystemSolver(const Config& config)
    : LinearSystemSolver<T>(config, "BGLinearSystemSolver"), n_iteration_{0}, L_{}, factors_{}, U_{} {}

template <class T>
Matrix<T> BgLinearSystemSolver<T>::B() const {
  Matrix<T> temp = P_.inverse() * L_;
  for (const auto& factor : factors_) temp *= factor;
  return temp *= U_;
}
template <class T>
Vector<T> BgLinearSystemSolver<T>::SolveCore(const Vector<T>& vector) const {
  DELPI_TRACE_FMT("BgLinearSystemSolver::SolveCore({})", vector);
  Matrix<T> temp = L_.template triangularView<Eigen::UnitLower>().solve(P_ * vector);
  for (const auto& factor : factors_) temp = factor.inverse() * temp;
  return U_.template triangularView<Eigen::Upper>().solve(temp);
}
template <class T>
Vector<T> BgLinearSystemSolver<T>::TransposeSolveCore(const Vector<T>& vector) const {
  DELPI_TRACE_FMT("BgLinearSystemSolver::TransposeSolveCore({})", vector);
  Matrix<T> temp = U_.template triangularView<Eigen::Upper>().transpose().solve(vector);
  for (const auto& factor : std::views::reverse(factors_)) temp = factor.transpose().inverse() * temp;
  return P_.inverse() * L_.template triangularView<Eigen::UnitLower>().transpose().solve(temp);
}
template <class T>
void BgLinearSystemSolver<T>::FactoriseCore(const Basis<T>& basis) {
  DELPI_TRACE_FMT("BgLinearSystemSolver::FactoriseCore({})", basis);

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
  const Eigen::PartialPivLU<Matrix<T>> lu_decomposition = basis.basis_vectors().partialPivLu();
  const Matrix<T>& LU_ = lu_decomposition.matrixLU();
  L_ = LU_.template triangularView<Eigen::UnitLower>().toDenseMatrix();
  U_ = LU_.template triangularView<Eigen::Upper>().toDenseMatrix();
  P_ = lu_decomposition.permutationP();
  // Reset any previous factors
  factors_.clear();
}
template <class T>
void BgLinearSystemSolver<T>::UpdateFactorisation(const Basis<T>& basis) {
  DELPI_ASSERT(L_.cols() > 0, "L must be initialized");
  DELPI_ASSERT(U_.cols() > 0, "U must be initialized");
  DELPI_ASSERT(basis.last_basis_leaving() <= basis.last_basis_entering(), "last_out must be less than last_in");
  DELPI_TRACE_FMT("UpdateFactorisation({})", basis);
  // Update the LU decomposition using the Bartels-Golub method
  // B = [B1, B2, ... Bl-1, Bl+1, ... Be]       where Bl is the last out and Be is the last in
  // H = [U1, U2, ... Ul-1, Ul+1, ... L^-1Be]   we create a hessemberg matrix H (but for efficiency we store it in U_)
  for (int i = basis.last_basis_leaving(); i < basis.last_basis_entering(); ++i) {
    U_.col(i).swap(U_.col(i + 1));
  }
  Matrix<T> LC = L_;
  for (const auto& factor : factors_) LC *= factor;
  U_.col(basis.last_basis_entering()) =
      LC.partialPivLu().solve(P_ * basis.basis_vectors().col(basis.last_basis_entering()));

  const Eigen::PartialPivLU<Matrix<T>> lu_decomposition = U_.partialPivLu();
  DELPI_ASSERT(U_.fullPivLu().determinant() != 0, "Determinant must be non-zero");

  U_ = lu_decomposition.matrixLU().template triangularView<Eigen::Upper>().toDenseMatrix();

  factors_.emplace_back(lu_decomposition.permutationP().inverse() *
                        lu_decomposition.matrixLU().template triangularView<Eigen::UnitLower>().toDenseMatrix());
#if 0
  std::cout << "H_\n" << U_ << std::endl;
  std::cout << "U_goal\n"
            << tempLU.permutationP().inverse() *
                   tempLU.matrixLU().template triangularView<Eigen::Upper>().toDenseMatrix()
            << std::endl;
  std::cout << "L_goal\n"
            << tempLU.permutationP().inverse() *
                   tempLU.matrixLU().template triangularView<Eigen::UnitLower>().toDenseMatrix()
            << std::endl;
  std::cout << "P_goal\n" << tempLU.permutationP().inverse().toDenseMatrix() << std::endl;

  // Lastly, we need to factorise the H matrix to get an upper triangular matrix U
  int counter = 0;
  for (int pivot = basis.last_basis_leaving(), to_update = basis.last_basis_leaving() + 1;
       to_update <= basis.last_basis_entering(); ++to_update) {
    const bool permuted = U_(to_update, pivot) > U_(pivot, pivot);
    T factor{permuted ? -U_(pivot, pivot) / U_(to_update, pivot) : -U_(to_update, pivot) / U_(pivot, pivot)};
    U_.row(to_update) += U_.row(pivot) * factor;
    pivot = permuted ? to_update : pivot;
    factors_.emplace_back(factor, to_update - 1, permuted);
    counter++;
  }

  for (const auto& factor : factors_) {
    std::cout << factor << std::endl;
    std::cout << "-------------------" << std::endl;
  }

  std::vector<RetriangularisationFactor<T>> reverse_factors;
  for (const auto& factor : std::ranges::views::reverse(factors_)) {
    if (!--counter) break;
    reverse_factors.emplace_back(factor.Inverse());
  }

  std::cout << "U_real\n" << U_.template triangularView<Eigen::Upper>().toDenseMatrix() << std::endl;
  std::cout << "U_reconstructed\n"
            << reverse_factors * U_.template triangularView<Eigen::Upper>().toDenseMatrix() << std::endl;
#endif
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
