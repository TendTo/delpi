#include "LinearSystemSolver.h"

namespace delpi::internal {
template <class T>
LinearSystemSolver<T>::LinearSystemSolver(const Config& config, const std::string& class_name)
    : config_{config}, stats_{config.with_timings(), class_name, "Total time spent in linear system solving"} {}
template <class T>
void LinearSystemSolver<T>::Factorise(const Basis<T>& basis) {
  TimerGuard timer_guard(&stats_.m_timer(), stats_.enabled());
  FactoriseCore(basis);
}
template <class T>
void LinearSystemSolver<T>::Reset() {
  TimerGuard timer_guard(&stats_.m_timer(), stats_.enabled());
  ResetCore();
}
template <class T>
Vector<T> LinearSystemSolver<T>::Solve(const Vector<T>& vector) const {
  TimerGuard timer_guard(&stats_.m_timer(), stats_.enabled());
  return SolveCore(vector);
}
template <class T>
Vector<T> LinearSystemSolver<T>::TransposeSolve(const Vector<T>& vector) const {
  TimerGuard timer_guard(&stats_.m_timer(), stats_.enabled());
  return TransposeSolveCore(vector);
}

template class LinearSystemSolver<mpq_class>;
template class LinearSystemSolver<double>;

}  // namespace delpi::internal
