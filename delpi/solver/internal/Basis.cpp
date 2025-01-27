
#include "Basis.h"

namespace delpi::internal {

template <class T>
Basis<T>::Basis(const Eigen::MatrixX<T>& A, std::vector<int> basis_idxs)
    : precision_{0},
      max_updates_before_refactor_{0},
      basis_idxs_{std::make_shared<std::vector<int>>(std::move(basis_idxs))},
      basis_vectors_{A(Eigen::all, *basis_idxs_)},
      last_in_{-1},
      last_out_{-1},
      last_idx_{-1} {}

template <class T>
Basis<T>::Basis(const Eigen::MatrixX<T>& A, const std::shared_ptr<std::vector<int>>& basis_idxs)
    : precision_{0},
      max_updates_before_refactor_{0},
      basis_idxs_{basis_idxs},
      basis_vectors_{A(Eigen::all, *basis_idxs_)},
      last_in_{-1},
      last_out_{-1},
      last_idx_{-1} {}

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