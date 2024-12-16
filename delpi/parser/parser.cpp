/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "parser.h"

#include "delpi/parser/mps/Driver.h"
#include "delpi/util/error.h"

namespace delpi {

bool LpSolver::Parse() { return config_.read_from_stdin() ? ParseStream(std::cin) : ParseFile(config_.filename()); }
bool LpSolver::ParseFile(const std::string& filename) { return GetDriverInstance(*this)->ParseFile(filename); }
bool LpSolver::ParseStream(std::istream& stream, const std::string& stream_name) {
  return GetDriverInstance(*this)->ParseStream(stream, stream_name);
}
bool LpSolver::ParseString(const std::string& string) { return GetDriverInstance(*this)->ParseString(string); }

std::unique_ptr<Driver> GetDriverInstance(LpSolver& lp_solver) {
  switch (lp_solver.config().actual_format()) {
    case Config::Format::MPS:
      return std::make_unique<mps::MpsDriver>(lp_solver);
    default:
      DELPI_UNREACHABLE();
  }
}

}  // namespace delpi
