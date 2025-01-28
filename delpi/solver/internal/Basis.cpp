
#include "delpi/solver/internal/Basis.h"

namespace delpi::internal {

template <class T>
Basis<T>::Basis(const Eigen::MatrixX<T>& A, std::vector<int> basis_idxs)
    : max_updates_before_refactor_{0},
      basis_idxs_{std::make_shared<std::vector<int>>(std::move(basis_idxs))},
      basis_vectors_{A(Eigen::all, *basis_idxs_)},
      last_basis_entering_{-1},
      last_basis_leaving_{-1},
      last_leaving_{-1},
      last_entering_{-1} {}

template <class T>
Basis<T>::Basis(const Eigen::MatrixX<T>& A, const std::shared_ptr<std::vector<int>>& basis_idxs)
    : max_updates_before_refactor_{0},
      basis_idxs_{basis_idxs},
      basis_vectors_{A(Eigen::all, *basis_idxs_)},
      last_basis_entering_{-1},
      last_basis_leaving_{-1},
      last_leaving_{-1},
      last_entering_{-1} {}
template <class T>
void Basis<T>::Update(const Eigen::MatrixX<T>& A, const int leaving, const int entering) {
  DELPI_TRACE_FMT("Basis::Update(leaving = {}, entering = {})", leaving, entering);
  last_basis_leaving_ = leaving;
  // TODO(tend): the place where the basis is updated could be customised by a subclass
  last_basis_entering_ = basis_vectors_.cols() - 1;
  last_leaving_ = basis_idxs_->at(leaving);
  last_entering_ = entering;

  // Move all columns to the right of the leaving column one position to the left
  if (leaving < basis_vectors_.cols() - 1) {
    std::rotate(basis_idxs_->begin() + leaving, basis_idxs_->begin() + leaving + 1, basis_idxs_->end());
    basis_idxs_->back() = entering;
    basis_vectors_ = A(Eigen::all, *basis_idxs_);
  }
}

template <class T>
std::ostream& operator<<(std::ostream& os, const Basis<T>& basis) {
  return os << "Basis " << basis.basis_vectors();
}

template class Basis<mpq_class>;
template class Basis<double>;
template class Basis<int>;
template class Basis<float>;

template std::ostream& operator<<(std::ostream& os, const Basis<mpq_class>& basis);
template std::ostream& operator<<(std::ostream& os, const Basis<double>& basis);
template std::ostream& operator<<(std::ostream& os, const Basis<int>& basis);
template std::ostream& operator<<(std::ostream& os, const Basis<float>& basis);

}  // namespace delpi::internal