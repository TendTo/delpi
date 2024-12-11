/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "Driver.h"

#include <fstream>
#include <iostream>
#include <sstream>  // IWYU pragma: keep for std::stringstream

#include "delpi/libs/gmp.h"
#include "delpi/util/Config.h"
#include "delpi/util/error.h"

namespace delpi {

Driver::Driver(LpSolver& lp_solver, const std::string& class_name)
    : lp_solver_{lp_solver}, stats_{lp_solver.config().with_timings(), class_name, "Total time spent in parsing"} {}

bool Driver::ParseStream(std::istream& in, const std::string& sname) {
  TimerGuard timer_guard(&stats_.m_timer(), stats_.enabled());
  stream_name_ = sname;
  return ParseStreamCore(in);
}

bool Driver::ParseString(const std::string& input, const std::string& sname) {
  std::istringstream iss(input);
  return ParseStream(iss, sname);
}

bool Driver::ParseFile(const std::string& filename) {
  std::ifstream in(filename.c_str());
  if (!in.good()) return false;
  return ParseStream(in, filename);
}

void Driver::Error(const std::string& m) { std::cerr << m << std::endl; }

void Driver::CheckSat() {
  // Don't consider the time spent checking sat in the time spent parsing.
  stats_.m_timer().Pause();
  mpq_class precision = lp_solver_.config().precision();
  lp_solver_.Solve(precision);
  stats_.m_timer().Resume();
}

void Driver::GetConstraints() const {
  if (lp_solver_.config().silent()) return;
  std::cout << "(constraints\n";
  for (const Formula& f : context_.assertions()) {
    std::cout << "\t" << f << "\n";
  }
  std::cout << ")" << std::endl;
}

void Driver::GetInfo(const std::string& key) const {
  if (lp_solver_.config().silent()) return;
  std::cout << "get-info ( " << key << " ): " << lp_solver_.GetInfo(key) << std::endl;
}
void Driver::SetInfo(const std::string& key, const std::string& value) { lp_solver_.SetInfo(key, value); }
void Driver::SetOption(const std::string& key, const std::string& value) { lp_solver_.SetOption(key, value); }

void Driver::Maximise(const Expression& objective_function) {
  // Don't consider the time spent checking sat in the time spent parsing.
  stats_.m_timer().Pause();
  lp_solver_.Maximise(objective_function);
  stats_.m_timer().Resume();
}

void Driver::Minimise(const Expression& objective_function) {
  // Don't consider the time spent checking sat in the time spent parsing.
  stats_.m_timer().Pause();
  lp_solver_.Minimise(objective_function);
  stats_.m_timer().Resume();
}

}  // namespace delpi
