#include "FpLpSolver.h"

#include "Factorisation.h"

namespace delpi::internal {
FpLpSolver::FpLpSolver(const Config& config, const std::string& class_name)
    : config_{config}, stats_{config_.with_timings(), class_name, "Time spent solving LP"} {}
void FpLpSolver::Solve(const EMatrix& A, const EVector& b, const EVector& c, const Basis& basis) {
  DMatrix A_d = A.cast<double>();
  DVector b_d = b.cast<double>();
  DVector c_d = c.cast<double>();
  Factorisation factorisation{config_, "Factorisation"};
  factorisation.Factorise(basis)
}

}  // namespace delpi::internal
