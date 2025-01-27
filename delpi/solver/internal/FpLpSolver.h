#pragma once

#include "Basis.h"
#include "delpi/libs/eigen.h"
#include "delpi/util/Config.h"
#include "delpi/util/Stats.h"

namespace delpi::internal {

class FpLpSolver {
 public:
  FpLpSolver(const Config& config, const std::string& class_name);
  void Solve(const EMatrix& A, const EVector& b, const EVector& c, const Basis& basis);

  const Config& config() const { return config_; }
  const IterationStats& stats() const { return stats_; }

 protected:
  const Config& config_;
  IterationStats stats_;
};

}  // namespace delpi::internal
