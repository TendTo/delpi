/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/solver/internal/Basis.h"

#include <numeric>
#include <ranges>
#include <unordered_set>

#include "delpi/util/error.h"
#include "delpi/util/logging.h"

namespace delpi::internal {

namespace {
std::vector<Index> ToVector(const Index rows, const Index cols) {
  std::vector<Index> result(rows);
  std::iota(result.begin(), result.end(), cols - rows);
  return result;
}
}  // namespace

template <IsAnyOf<mpq_class, double> T>
Basis<T>::Basis(const Eigen::MatrixX<T>& A) : Basis(A, ToVector(A.rows(), A.cols())) {}

template <IsAnyOf<mpq_class, double> T>
Basis<T>::Basis(const Eigen::MatrixX<T>& A, std::vector<Index> basis_idxs)
    : Basis(A, std::make_shared<std::vector<Index>>(std::move(basis_idxs))) {}

template <IsAnyOf<mpq_class, double> T>
Basis<T>::Basis(const Eigen::MatrixX<T>& A, const std::shared_ptr<std::vector<Index>>& basis_idxs)
    : A_{A},
      basis_vectors_{A(Eigen::placeholders::all, *basis_idxs)},
      basis_idxs_{basis_idxs},
      last_basis_entering_{-1},
      last_basis_leaving_{-1},
      last_leaving_{-1},
      last_entering_{-1} {
  DELPI_ASSERT(static_cast<std::size_t>(basis_vectors_.cols()) == basis_idxs_->size(),
               "Basis vectors and indices must have the same size");
  DELPI_ASSERT(std::ranges::all_of(*basis_idxs_, [this](const int idx) { return idx >= 0 && idx < A_.cols(); }),
               "All indices must be valid");
  DELPI_ASSERT(std::unordered_set(basis_idxs_->begin(), basis_idxs_->end()).size() == basis_idxs_->size(),
               "All indices must be unique");
}

template <IsAnyOf<mpq_class, double> T>
template <IsAnyOf<mpq_class, double> M>
Basis<T>& Basis<T>::operator=(const Basis<M>& basis) {
  *this = Basis<T>{A_, basis.basis_idxs()};
  return *this;
}
template <IsAnyOf<mpq_class, double> T>
template <IsAnyOf<mpq_class, double> M>
Basis<T>& Basis<T>::FromBasis(const Basis<M>& basis, const std::vector<std::size_t>& col_to_remove) {
  DELPI_TRACE("Basis::FromBasis()");
  if (basis_idxs_.use_count() > 1) basis_idxs_ = std::make_shared<std::vector<Index>>();
  basis_idxs_->clear();
  basis_idxs_->reserve(basis.size() - col_to_remove.size());
  std::size_t inserted_idx = 0;
  for (const std::size_t removed_col : col_to_remove) {
    basis_idxs_->insert(basis_idxs_->end(), basis.basis_idxs().begin() + inserted_idx,
                        basis.basis_idxs().begin() + removed_col);
    inserted_idx = removed_col + 1;
  }
  basis_idxs_->insert(basis_idxs_->end(), basis.basis_idxs().begin() + inserted_idx, basis.basis_idxs().end());
  DELPI_ASSERT(std::ranges::all_of(*basis_idxs_, [this](const Index idx) { return idx >= 0 && idx < A_.cols(); }),
               "All indices must be valid");
  DELPI_ASSERT(std::unordered_set(basis_idxs_->begin(), basis_idxs_->end()).size() == basis_idxs_->size(),
               "All indices must be unique");
  basis_vectors_ = A_(Eigen::placeholders::all, *basis_idxs_);
  return *this;
}
template <IsAnyOf<mpq_class, double> T>
Basis<T>& Basis<T>::operator=(const Basis<T>& basis) {
  if (this == &basis) return *this;
  DELPI_ASSERT(std::ranges::all_of(basis.basis_idxs(), [this](const int idx) { return idx >= 0 && idx < A_.cols(); }),
               "All indices must be valid");
  DELPI_ASSERT(
      std::unordered_set(basis.basis_idxs().begin(), basis.basis_idxs().end()).size() == basis.basis_idxs().size(),
      "All indices must be unique");
  basis_idxs_ = basis.basis_idxs_;
  basis_vectors_ = A_(Eigen::placeholders::all, *basis_idxs_);
  return *this;
}

template <IsAnyOf<mpq_class, double> T>
void Basis<T>::Update(const Index leaving, const Index entering) {
  DELPI_ASSERT(leaving >= 0 && leaving < A_.rows(), "Invalid leaving index");
  DELPI_ASSERT(entering >= 0 && entering < A_.cols(), "Invalid entering index");
  DELPI_TRACE_FMT("Basis::Update(leaving = {}, entering = {})", leaving, entering);

  // Ensure that no other instance sharing the same basis indexes is affected
  if (basis_idxs_.use_count() > 1) basis_idxs_ = std::make_shared<std::vector<Index>>(*basis_idxs_);

  last_basis_leaving_ = leaving;
  // TODO(tend): the place where the basis is updated could be customised by a subclass
  last_basis_entering_ = basis_vectors_.cols() - 1;
  last_leaving_ = basis_idxs_->at(leaving);
  last_entering_ = entering;

  // Move all columns to the right of the leaving column one position to the left
  std::rotate(basis_idxs_->begin() + leaving, basis_idxs_->begin() + leaving + 1, basis_idxs_->end());
  // Set the last basis column to the entering column
  basis_idxs_->back() = entering;
  basis_vectors_ = A_(Eigen::placeholders::all, *basis_idxs_);
}
template <IsAnyOf<mpq_class, double> T>
void Basis<T>::OffsetIndexes(const Index offset) {
  // Ensure that no other instance sharing the same basis indexes is affected
  if (basis_idxs_.use_count() > 1) basis_idxs_ = std::make_shared<std::vector<Index>>(*basis_idxs_);
  for (auto& idx : *basis_idxs_) idx += offset;
  basis_vectors_ = A_(Eigen::placeholders::all, *basis_idxs_);
}

template <IsAnyOf<mpq_class, double> T>
std::ostream& operator<<(std::ostream& os, const Basis<T>& basis) {
  return os << basis.basis_vectors();
}

template class Basis<mpq_class>;
template class Basis<double>;

template Basis<mpq_class>& Basis<mpq_class>::operator=(const Basis<double>& basis);
template Basis<double>& Basis<double>::operator=(const Basis<mpq_class>& basis);

template Basis<mpq_class>& Basis<mpq_class>::FromBasis(const Basis<mpq_class>& basis,
                                                       const std::vector<std::size_t>& col_to_remove);
template Basis<mpq_class>& Basis<mpq_class>::FromBasis(const Basis<double>& basis,
                                                       const std::vector<std::size_t>& col_to_remove);
template Basis<double>& Basis<double>::FromBasis(const Basis<mpq_class>& basis,
                                                 const std::vector<std::size_t>& col_to_remove);
template Basis<double>& Basis<double>::FromBasis(const Basis<double>& basis,
                                                 const std::vector<std::size_t>& col_to_remove);

template std::ostream& operator<<(std::ostream& os, const Basis<mpq_class>& basis);
template std::ostream& operator<<(std::ostream& os, const Basis<double>& basis);

}  // namespace delpi::internal
