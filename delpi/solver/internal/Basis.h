/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Basis class.
 */
#pragma once

#include <iosfwd>
#include <memory>
#include <vector>

#include "delpi/libs/eigen.h"
#include "delpi/libs/gmp.h"

namespace delpi::internal {

/**
 * Simplex basis.
 * Consider a standard LP problem:
 * @f[
 * \begin{array}{rl}
 * \hbox{max}  & c^T x                 \\
 * \hbox{s.t.} & l_r \le Ax \le u_r    \\
 * & l_c \le  x \le u_c
 * \end{array}
 * @f]
 * where @f$c, l_c, u_c, x \in {\bf R}^n @f$, @f$ l_r, u_r \in {\bf R}^m @f$ and @f$ A \in {\bf R}^{m \times n} @f$.
 * Solving this LP with the simplex algorithm requires the definition of a **basis**,
 * a set of column vectors or a set of row vectors building a non-singular matrix.
 * We will refer to the first case as the **columnwise** representation
 * and the latter case will be called the **rowwise** representation.
 * The size of the vectors is referred to as the basis' **size**.
 *
 * This class represents a generic simplex basis, suitable for both representations.
 * At any time the representation can be changed using the method @ref SwitchRep().
 *
 * This class provides methods for solving linear systems with the basis matrix.
 * However, SPxBasisBase does not provide a linear solver by its own.
 * Instead, a SLinSolver object must be loaded to a SPxBasisBase which will be called for solving linear systems.
 */
// TODO(tend): fix reference to coefficient matrix
template <class T>
class Basis {
 public:
  using BasisVectors = decltype(Eigen::MatrixX<T>(Eigen::all, std::declval<std::vector<int>>()));
  explicit Basis(const Eigen::MatrixX<T>& A);
  Basis(const Eigen::MatrixX<T>& A, std::vector<int> basis_idxs);
  Basis(const Eigen::MatrixX<T>& A, const std::shared_ptr<std::vector<int>>& basis_idxs);
  template <class M>
  Basis(const Eigen::MatrixX<T>& A, const Basis<M>& basis) : Basis{A, basis.basis_idxs_} {}

  [[nodiscard]] const BasisVectors& basis_vectors() const { return basis_vectors_; }
  [[nodiscard]] const std::vector<int>& basis_idxs() const { return *basis_idxs_; }
  [[nodiscard]] int size() const { return basis_idxs_->size(); }
  [[nodiscard]] int last_basis_entering() const { return last_basis_entering_; }
  [[nodiscard]] int last_basis_leaving() const { return last_basis_leaving_; }
  [[nodiscard]] int last_leaving() const { return last_leaving_; }
  [[nodiscard]] int last_entering() const { return last_entering_; }

  void Update(const Eigen::MatrixX<T>& A, int leaving, int entering);
  void OffsetIndexes(int offset);

 protected:
  int max_updates_before_refactor_;               ///< Number of updates before a forced refactorization of the basis
  std::shared_ptr<std::vector<int>> basis_idxs_;  ///< Indices of the basis vectors
  BasisVectors basis_vectors_;                    ///< Basis columns taken from A

  int last_basis_entering_;  ///< Index of where the latest column was added in the basis
  int last_basis_leaving_;   ///< Index of where the latest column was removed from the basis
  int last_leaving_;         ///< Index of the variable that left the basis last. Relative to the original matrix
  int last_entering_;        ///< Index of the variable that entered the basis last. Relative to the original matrix
};

template <class T>
std::ostream& operator<<(std::ostream& os, const Basis<T>& basis);

using EBasis = Basis<mpq_class>;
using DBasis = Basis<double>;

}  // namespace delpi::internal

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::internal::Basis<mpq_class>)
OSTREAM_FORMATTER(delpi::internal::Basis<double>)

#endif
