#include "Factorisation.h"

namespace delpi::internal {
Factorisation::Factorisation(const Config& config, const std::string& class_name)
    : config_{config}, stats_{config_.with_timings(), class_name, "Time spent factorising basis"} {}
void Factorisation::Factorise(const Basis& basis) {
  TimerGuard timer_guard{&stats_.m_timer(), stats_.enabled()};
  FactoriseCore(basis);
}
void Factorisation::Reset() { ResetCore(); }

}  // namespace delpi::internal
