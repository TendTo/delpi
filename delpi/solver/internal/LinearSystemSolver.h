#pragma once
#include <string>

#include "delpi/libs/eigen.h"
#include "delpi/libs/gmp.h"
#include "delpi/solver/internal/Basis.h"
#include "delpi/util/Config.h"
#include "delpi/util/Stats.h"

namespace delpi::internal {

template <class T>
class LinearSystemSolver {
 public:
  explicit LinearSystemSolver(const Config& config, const std::string& class_name = "LinearSystemSolver");
  virtual ~LinearSystemSolver() = default;

  [[nodiscard]] const IterationStats& stats() const { return stats_; }
  [[nodiscard]] const Config& config() const { return config_; }

  void Factorise(const Basis<T>& basis);
  void Reset();

  [[nodiscard]] Vector<T> Solve(const Vector<T>& vector) const;
  [[nodiscard]] Vector<T> TransposeSolve(const Vector<T>& vector) const;

 protected:
  virtual Vector<T> SolveCore(const Vector<T>& vector) const = 0;
  virtual Vector<T> TransposeSolveCore(const Vector<T>& vector) const = 0;
  virtual void FactoriseCore(const Basis<T>& basis) = 0;
  virtual void ResetCore() = 0;

  const Config& config_;
  mutable IterationStats stats_;
};

using ELinearSystemSolver = LinearSystemSolver<mpq_class>;
using DLinearSystemSolver = LinearSystemSolver<double>;

}  // namespace delpi::internal
