#pragma once

#include "Basis.h"
#include "delpi/util/Config.h"
#include "delpi/util/Stats.h"

namespace delpi::internal {

class Factorisation {
 public:
  explicit Factorisation(const Config& config, const std::string& class_name = "Factorisation");
  virtual ~Factorisation() = default;

  void Factorise(const Basis& basis);

  void Reset();

  const IterationStats& stats() const { return stats_; }
  const Config& config() const { return config_; }
  const std::vector<int> Solve(std::vector<int> vector);
  const std::vector<int> TransposeSolve(std::vector<int> vector);

 protected:
  virtual void FactoriseCore(const Basis& basis) = 0;
  virtual void ResetCore() = 0;

  const Config& config_;
  IterationStats stats_;
};

}  // namespace delpi::internal
