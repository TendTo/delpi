/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "delpi/parser/parser.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "delpi/parser/mps/Driver.h"
#include "delpi/util/error.h"

namespace delpi {

bool LpSolver::Parse() { return config_.read_from_stdin() ? ParseStream(std::cin) : ParseFile(config_.filename()); }
bool LpSolver::ParseFile(const std::string& filename) {
  std::ifstream in(filename.c_str());
  if (!in.good()) return false;
  return ParseStream(in, filename);
}
bool LpSolver::ParseStream(std::istream& stream, const std::string& stream_name) {
  const std::unique_ptr parser{GetDriverInstance(*this)};
  DELPI_ASSERT(parser, "Parser not found");
  const bool res = parser->ParseStream(stream, stream_name);
  stats_.parser_stats = parser->stats();
  return res;
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
