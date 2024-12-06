/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024
 * @licence Apache-2.0 license
 */
#include "delpi/utils/calculator.h"

namespace delpi {

Calculator::Calculator(int verbose) : verbose_{verbose} {
  // Se the level of the logger globally.
  // Not the recommended for serious code.
  switch (verbose_) {
    case 1:
      spdlog::set_level(spdlog::level::info);
      break;
    case 2:
      spdlog::set_level(spdlog::level::debug);
      break;
    default:
      spdlog::set_level(spdlog::level::off);
      break;
  }
  spdlog::info("Calculator initialized with verbose level: {}", verbose_);
}

int Calculator::getVerbose() {
  spdlog::debug("Verbose level: {}", verbose_);
  return verbose_;
}

}  // namespace delpi
