
#include "delpi/solver/internal/Basis.h"

#include <numeric>
#include <ranges>

#include "delpi/util/error.h"

namespace delpi::internal {

namespace {
std::vector<int> ToVector(const int size) {
  std::vector<int> result(size);
  std::iota(result.begin(), result.end(), 0);
  return result;
}
}  // namespace

template <class T>
Basis<T>::Basis(const Eigen::MatrixX<T>& A) : Basis(A, ToVector(A.rows())) {}

template <class T>
Basis<T>::Basis(const Eigen::MatrixX<T>& A, std::vector<int> basis_idxs)
    : Basis(A, std::make_shared<std::vector<int>>(std::move(basis_idxs))) {}

template <class T>
Basis<T>::Basis(const Eigen::MatrixX<T>& A, const std::shared_ptr<std::vector<int>>& basis_idxs)
    : max_updates_before_refactor_{0},
      basis_idxs_{basis_idxs},
      basis_vectors_{A(Eigen::all, *basis_idxs_)},
      last_basis_entering_{-1},
      last_basis_leaving_{-1},
      last_leaving_{-1},
      last_entering_{-1} {
  DELPI_ASSERT(static_cast<std::size_t>(basis_vectors_.cols()) == basis_idxs_->size(),
               "Basis vectors and indices must have the same size");
}

template <class T>
void Basis<T>::Update(const Eigen::MatrixX<T>& A, const int leaving, const int entering) {
  DELPI_ASSERT(leaving >= 0 && leaving < A.rows(), "Invalid leaving index");
  DELPI_ASSERT(entering >= 0 && entering < A.cols(), "Invalid entering index");
  DELPI_TRACE_FMT("Basis::Update(leaving = {}, entering = {})", leaving, entering);

  // Ensure that no other instance sharing the same basis indexes is affected
  if (basis_idxs_.use_count() > 1) basis_idxs_ = std::make_shared<std::vector<int>>(*basis_idxs_);

  last_basis_leaving_ = leaving;
  // TODO(tend): the place where the basis is updated could be customised by a subclass
  last_basis_entering_ = basis_vectors_.cols() - 1;
  last_leaving_ = basis_idxs_->at(leaving);
  last_entering_ = entering;

  // Move all columns to the right of the leaving column one position to the left
  std::rotate(basis_idxs_->begin() + leaving, basis_idxs_->begin() + leaving + 1, basis_idxs_->end());
  // Set the last basis column to the entering column
  basis_idxs_->back() = entering;
  basis_vectors_ = A(Eigen::all, *basis_idxs_);
}
template <class T>
void Basis<T>::OffsetIndexes(const int offset) {
  // Ensure that no other instance sharing the same basis indexes is affected
  if (basis_idxs_.use_count() > 1) basis_idxs_ = std::make_shared<std::vector<int>>(*basis_idxs_);
  for (auto& idx : *basis_idxs_) idx += offset;
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