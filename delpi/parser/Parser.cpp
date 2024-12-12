/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */

#include "Parser.h"

#include <fstream>

#include "delpi/parser/mps/Driver.h"
#include "delpi/util/error.h"

namespace delpi {

bool LpSolver::ParseFile(const std::string& filename) {
  return Parser(*this).GetDriverInstance(*this)->ParseFile(filename);
}
bool LpSolver::ParseStream(std::istream& stream) { return Parser(*this).GetDriverInstance(*this)->ParseStream(stream); }
bool LpSolver::ParseString(const std::string& string) {
  return Parser(*this).GetDriverInstance(*this)->ParseString(string);
}

Parser::Parser(LpSolver& lp_solver) : driver_(GetDriverInstance(lp_solver)) {}

std::unique_ptr<Driver> Parser::GetDriverInstance(LpSolver& lp_solver) {
  switch (lp_solver.config().actual_format()) {
    case Config::Format::MPS:
      return std::make_unique<mps::MpsDriver>(lp_solver);
    default:
      DELPI_UNREACHABLE();
  }
}

const Stats& Parser::stats() const { return driver_->stats(); }

}  // namespace delpi