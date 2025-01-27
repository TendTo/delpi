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
template <class T>
class Basis {
 public:
  using BasisVectors = decltype(Eigen::MatrixX<T>(Eigen::all, std::declval<std::vector<int>>()));
  explicit Basis(const Eigen::MatrixX<T>& A, std::vector<int> basis_idxs = {});
  explicit Basis(const Eigen::MatrixX<T>& A, const std::shared_ptr<std::vector<int>>& basis_idxs);
  template <class M>
  explicit Basis(const Eigen::MatrixX<T>& A, const Basis<M>& basis) : Basis{A, basis.basis_idxs_} {}

  [[nodiscard]] const BasisVectors& basis_vectors() const { return basis_vectors_; }
  [[nodiscard]] const std::vector<int>& basis_idxs() const { return *basis_idxs_; }
  [[nodiscard]] int size() const { return basis_idxs_->size(); }
  [[nodiscard]] int last_in() const { return last_in_; }
  [[nodiscard]] int last_out() const { return last_out_; }

 protected:
  /** Each basis is assigned a status flag. */
  // enum class SPxStatus {
  //   UNSOLVED = -2,  ///< No Problem has been loaded to the basis
  //   SINGULAR = -1,  ///< Basis is singular
  //   REGULAR = 0,    ///< Basis is not known to be dual nor primal feasible
  //   DUAL = 1,       ///< Basis is dual feasible
  //   PRIMAL = 2,     ///< Basis is primal feasible
  //   OPTIMAL = 3,    ///< Basis is optimal, i.e. dual and primal feasible
  //   UNBOUNDED = 4,  ///< LP has been proven to be primal unbounded
  //   INFEASIBLE = 5  ///< LP has been proven to be primal infeasible
  // };
  int precision_;  ///< Precision of the basis. 0 means we are dealing with rationals

  int max_updates_before_refactor_;               ///< Number of updates before a forced refactorization of the basis
  std::shared_ptr<std::vector<int>> basis_idxs_;  ///< Indices of the basis vectors
  BasisVectors basis_vectors_;                    ///< Basis columns taken from A

  int last_in_;   ///< lastEntered(): variable entered the base last
  int last_out_;  ///< lastLeft(): variable left the base last
  int last_idx_;  ///< lastIndex(): basis index where last update was done
  // mpq_class min_stability_;  ///< minimum stability
};

template <class T>
std::ostream& operator<<(std::ostream& os, const Basis<T>& basis);

}  // namespace delpi::internal
