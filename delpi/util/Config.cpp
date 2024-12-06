/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
#include "Config.h"

#include <ostream>
#include <utility>

#include "delpi/util/error.h"
#include "delpi/util/filesystem.h"

namespace delpi {
Config::Config(std::string filename) : filename_{std::move(filename)} {}
Config::Config(const bool read_from_stdin) : read_from_stdin_{read_from_stdin} {}
Config::Config(const Format format) : format_{format} {}

std::string Config::filename_extension() const { return GetExtension(filename_.get()); }

Config::LPMode Config::actual_lp_mode() const {
  switch (lp_mode_.get()) {
    case LPMode::AUTO:
      return lp_solver_.get() == LPSolver::QSOPTEX ? LPMode::PURE_PRECISION_BOOSTING : LPMode::HYBRID;
    default:
      return lp_mode_.get();
  }
}
Config::Format Config::actual_format() const {
  switch (format_.get()) {
    case Format::AUTO:
      if (filename_extension() == "mps") {
        return Format::MPS;
      }
      DELPI_RUNTIME_ERROR("Cannot determine format from stdin or unknown file extension");
    default:
      return format_.get();
  }
}

std::ostream &operator<<(std::ostream &os, const Config::LPSolver &lp_solver) {
  switch (lp_solver) {
    case Config::LPSolver::QSOPTEX:
      return os << "qsoptex";
    case Config::LPSolver::SOPLEX:
      return os << "soplex";
    default:
      DELPI_UNREACHABLE();
  }
}

std::ostream &operator<<(std::ostream &os, const Config::Format &format) {
  switch (format) {
    case Config::Format::AUTO:
      return os << "auto";
    case Config::Format::MPS:
      return os << "mps";
    default:
      DELPI_UNREACHABLE();
  }
}

std::ostream &operator<<(std::ostream &os, const Config::LPMode &mode) {
  switch (mode) {
    case Config::LPMode::AUTO:
      return os << "A";
    case Config::LPMode::PURE_PRECISION_BOOSTING:
      return os << "P";
    case Config::LPMode::PURE_ITERATIVE_REFINEMENT:
      return os << "I";
    case Config::LPMode::HYBRID:
      return os << "H";
    default:
      DELPI_UNREACHABLE();
  }
}

std::ostream &operator<<(std::ostream &os, const Config &config) {
  return os << "Config {\n"
            << "csv = " << config.csv() << ",\n"
            << "continuous_output = " << config.continuous_output() << ",\n"
            << "debug_parsing = " << config.debug_parsing() << ",\n"
            << "debug_scanning = " << config.debug_scanning() << ",\n"
            << "filename = '" << config.filename() << "',\n"
            << "format = '" << config.format() << "',\n"
            << "lp_mode = '" << config.lp_mode() << "',\n"
            << "lp_solver = " << config.lp_solver() << ",\n"
            << "number_of_jobs = " << config.number_of_jobs() << ",\n"
            << "optimize = '" << config.optimize() << "',\n"
            << "precision = " << config.precision() << ",\n"
            << "produce_model = " << config.produce_models() << ",\n"
            << "random_seed = " << config.random_seed() << ",\n"
            << "read_from_stdin = " << config.read_from_stdin() << ",\n"
            << "silent = " << config.silent() << ",\n"
            << "timeout = " << config.timeout() << ",\n"
            << "verbose_delpi = " << config.verbose_delpi() << ",\n"
            << "verbose_simplex = " << config.verbose_simplex() << ",\n"
            << "verify = " << config.verify() << ",\n"
            << "with_timings = " << config.with_timings() << ",\n"
            << '}';
}

}  // namespace delpi
