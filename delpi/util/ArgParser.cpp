/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 */
// IWYU pragma: no_include "argparse/argparse.hpp" // Already included in the header
#include "ArgParser.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#ifdef DELPI_ENABLED_QSOPTEX
#include "delpi/libs/qsopt_ex.h"
#endif
#ifdef DELPI_ENABLED_SOPLEX
#include "delpi/libs/soplex.h"
#endif

#include "delpi/util/OptionValue.hpp"
#include "delpi/util/error.h"
#include "delpi/util/filesystem.h"
#include "delpi/util/logging.h"
#include "delpi/version.h"

namespace delpi {

#define DELPI_PARSE_PARAM_BOOL(parser, name, ...)        \
  do {                                                   \
    parser.add_argument(__VA_ARGS__)                     \
        .help(delpi::Config::help_##name)                \
        .default_value(delpi::Config::default_##name)    \
        .implicit_value(!delpi::Config::default_##name); \
  } while (false)

#define DELPI_PARSE_PARAM_SCAN(parser, name, scan_char, scan_type, ...) \
  do {                                                                  \
    parser.add_argument(__VA_ARGS__)                                    \
        .help(delpi::Config::help_##name)                               \
        .default_value(delpi::Config::default_##name)                   \
        .nargs(1)                                                       \
        .scan<scan_char, scan_type>();                                  \
  } while (false)

#define DELPI_PARSE_PARAM_ENUM(parser, name, cmd_name, invalid_prompt, ...) \
  do {                                                                      \
    parser.add_argument(cmd_name)                                           \
        .help(delpi::Config::help_##name)                                   \
        .default_value(delpi::Config::default_##name)                       \
        .action([](const std::string &value) {                              \
          __VA_ARGS__                                                       \
          DELPI_INVALID_ARGUMENT_EXPECTED(cmd_name, value, invalid_prompt); \
        })                                                                  \
        .nargs(1);                                                          \
  } while (false)

#define DELPI_PARAM_TO_CONFIG(param_name, config_name, type)                                                     \
  do {                                                                                                           \
    if (parser_.is_used(param_name)) config.m_##config_name().SetFromCommandLine(parser_.get<type>(param_name)); \
  } while (false)

ArgParser::ArgParser()
    : parser_{DELPI_PROGRAM_NAME, DELPI_VERSION_STRING},
      verbosity_{Config::default_verbose_delpi}
#ifdef DELPI_ENABLED_QSOPTEX
      ,
      qsoptex_hash_{QSopt_ex_repository_status()}
#endif
#ifdef DELPI_ENABLED_SOPLEX
      ,
      soplex_hash_{soplex::getGitHash()}  // NOLINT(whitespace/braces)
#endif
{
  DELPI_TRACE("ArgParser::ArgParser");
  AddOptions();
}

void ArgParser::Parse(int argc, const char **argv) {
  try {
    parser_.parse_args(argc, argv);
    DELPI_LOG_INIT_VERBOSITY((parser_.get<bool>("silent") ? 0 : verbosity_));
    ValidateOptions();
    DELPI_TRACE("ArgParser::parse: parsed args");
  } catch (const std::runtime_error &err) {
    std::cerr << err.what() << "\n" << parser_ << std::endl;
    std::exit(EXIT_FAILURE);
  } catch (const std::invalid_argument &err) {
    std::cerr << err.what() << "\n\n" << parser_.usage() << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

void ArgParser::AddOptions() {
  DELPI_TRACE("ArgParser::AddOptions: adding options");
  parser_.add_description(prompt());
  parser_.add_argument("file").help("input file").default_value("");
  parser_.add_argument("--onnx-file").help("ONNX file name").default_value("").nargs(1);

  DELPI_PARSE_PARAM_BOOL(parser_, csv, "--csv");
  DELPI_PARSE_PARAM_BOOL(parser_, continuous_output, "--continuous-output");
  DELPI_PARSE_PARAM_BOOL(parser_, debug_parsing, "--debug-parsing");
  DELPI_PARSE_PARAM_BOOL(parser_, debug_scanning, "--debug-scanning");
  DELPI_PARSE_PARAM_BOOL(parser_, optimize, "-o", "--optimize");
  DELPI_PARSE_PARAM_BOOL(parser_, produce_models, "-m", "--produce-models");
  DELPI_PARSE_PARAM_BOOL(parser_, silent, "-s", "--silent");
  DELPI_PARSE_PARAM_BOOL(parser_, with_timings, "-t", "--timings");
  DELPI_PARSE_PARAM_BOOL(parser_, read_from_stdin, "--in");
  DELPI_PARSE_PARAM_BOOL(parser_, verify, "--verify");

  //  DELPI_PARSE_PARAM_SCAN(parser_, number_of_jobs, 'i', unsigned int, "-j", "--jobs");
  DELPI_PARSE_PARAM_SCAN(parser_, precision, 'g', double, "-p", "--precision");
  DELPI_PARSE_PARAM_SCAN(parser_, random_seed, 'i', unsigned int, "-r", "--random-seed");
  DELPI_PARSE_PARAM_SCAN(parser_, timeout, 'i', unsigned int, "--timeout");
  DELPI_PARSE_PARAM_SCAN(parser_, verbose_simplex, 'i', int, "--verbose-simplex");

  parser_.add_argument("-V", "--verbose")
      .help("increase verbosity level. Can be used multiple times. Maximum verbosity level is 5 and default is 2")
      .action([this](const auto &) {
        if (verbosity_ < 5) ++verbosity_;
      })
      .append()
      .nargs(0);
  parser_.add_argument("-q", "--quiet")
      .help("decrease verbosity level. Can be used multiple times. Minimum verbosity level is 0 and default is 2")
      .action([this](const auto &) {
        if (verbosity_ > 0) --verbosity_;
      })
      .append()
      .nargs(0);

  DELPI_PARSE_PARAM_ENUM(
      parser_, lp_mode, "--lp-mode",
      "[ auto | pure-precision-boosting | pure-iterative-refinement | hybrid ] or [ 1 | 2 | 3 | 4 ]",
      if (value == "auto" || value == "1") return Config::LPMode::AUTO;
      if (value == "pure-precision-boosting" || value == "2") return Config::LPMode::PURE_PRECISION_BOOSTING;
      if (value == "pure-iterative-refinement" || value == "3") return Config::LPMode::PURE_ITERATIVE_REFINEMENT;
      if (value == "hybrid" || value == "4") return Config::LPMode::HYBRID;);
  DELPI_PARSE_PARAM_ENUM(parser_, format, "--format", "[ auto | smt2 | mps  | vnnlib ] or [ 1 | 2 | 3 | 4 ]",
                         if (value == "auto" || value == "1") return Config::Format::AUTO;
                         if (value == "mps" || value == "2") return Config::Format::MPS;);
  DELPI_PARSE_PARAM_ENUM(parser_, lp_solver, "--lp-solver", "[ soplex | qsoptex ] or [ 1 | 2 ]",
                         if (value == "soplex" || value == "1") return Config::LPSolver::SOPLEX;
                         if (value == "qsoptex" || value == "2") return Config::LPSolver::QSOPTEX;);
  DELPI_TRACE("ArgParser::ArgParser: added all arguments");
}

Config ArgParser::ToConfig() const {
  DELPI_TRACE("ArgParser::ToConfig: converting to Config");
  Config config{};

  DELPI_PARAM_TO_CONFIG("csv", csv, bool);
  DELPI_PARAM_TO_CONFIG("continuous-output", continuous_output, bool);
  DELPI_PARAM_TO_CONFIG("debug-parsing", debug_parsing, bool);
  DELPI_PARAM_TO_CONFIG("debug-scanning", debug_scanning, bool);
  config.m_filename().SetFromCommandLine(parser_.is_used("file") ? parser_.get<std::string>("file") : "");
  DELPI_PARAM_TO_CONFIG("format", format, Config::Format);
  DELPI_PARAM_TO_CONFIG("lp-mode", lp_mode, Config::LPMode);
  DELPI_PARAM_TO_CONFIG("lp-solver", lp_solver, Config::LPSolver);
  // DELPI_PARAM_TO_CONFIG("jobs", number_of_jobs, unsigned int);
  DELPI_PARAM_TO_CONFIG("optimize", optimize, bool);
  DELPI_PARAM_TO_CONFIG("precision", precision, double);
  DELPI_PARAM_TO_CONFIG("produce-models", produce_models, bool);
  DELPI_PARAM_TO_CONFIG("random-seed", random_seed, unsigned int);
  DELPI_PARAM_TO_CONFIG("in", read_from_stdin, bool);
  DELPI_PARAM_TO_CONFIG("silent", silent, bool);
  DELPI_PARAM_TO_CONFIG("timeout", timeout, unsigned int);
  config.m_verbose_delpi().SetFromCommandLine(verbosity_);
  DELPI_PARAM_TO_CONFIG("verbose-simplex", verbose_simplex, int);
  DELPI_PARAM_TO_CONFIG("verify", verify, bool);
  DELPI_PARAM_TO_CONFIG("timings", with_timings, bool);

  DELPI_TRACE_FMT("ArgParser::ToConfig: {}", config);
  return config;
}

void ArgParser::ValidateOptions() {
  DELPI_TRACE("ArgParser::ValidateOptions: validating options");
  if (parser_.is_used("in") && parser_.is_used("file"))
    DELPI_INVALID_ARGUMENT("--in", "--in and file are mutually exclusive");
  if (!parser_.is_used("in") && !parser_.is_used("file"))
    DELPI_INVALID_ARGUMENT("file", "must be specified unless --in is used");
  if (parser_.is_used("in") && (parser_.get<Config::Format>("format") == Config::Format::AUTO))
    DELPI_INVALID_ARGUMENT("--in", "a format must be specified with --format");
  // Check file extension if a file is provided
  if (parser_.is_used("file")) {
    const Config::Format format = parser_.get<Config::Format>("format");
    const std::string extension{GetExtension(parser_.get<std::string>("file"))};
    if (format == Config::Format::AUTO && extension != "mps") {
      DELPI_INVALID_ARGUMENT("file", "file must be .mps if --format is auto");
    }
    if (!std::filesystem::is_regular_file(parser_.get<std::string>("file")))
      DELPI_INVALID_ARGUMENT("file", "cannot find file or the file is not a regular file");
  }
  if (parser_.get<double>("precision") < 0) DELPI_INVALID_ARGUMENT("--precision", "cannot be negative");
  if (parser_.is_used("verbose") && parser_.is_used("silent"))
    DELPI_INVALID_ARGUMENT("--verbose", "verbosity is forcefully set to 0 if --silent is provided");
  if (parser_.is_used("quiet") && parser_.is_used("silent"))
    DELPI_INVALID_ARGUMENT("--quiet", "verbosity is already set to 0 if --silent is provided");
  if (parser_.get<Config::LPSolver>("lp-solver") == Config::LPSolver::QSOPTEX)
    if (parser_.get<Config::LPMode>("lp-mode") != Config::LPMode::AUTO &&
        parser_.get<Config::LPMode>("lp-mode") != Config::LPMode::PURE_PRECISION_BOOSTING)
      DELPI_INVALID_ARGUMENT("--lp-solver", "QSopt_ex only supports 'auto' and 'pure-precision-boosting' modes");
}

std::string ArgParser::version() { return DELPI_VERSION_STRING; }

std::string ArgParser::repository_status() { return DELPI_VERSION_REPOSTAT; }

std::string ArgParser::prompt() const {
#ifndef NDEBUG
  const std::string build_type{"Debug"};
#else
  const std::string build_type{"Release"};
#endif
  std::string repo_stat = repository_status();
  if (!repo_stat.empty()) {
    repo_stat = " (repository: " + repo_stat + ")";
  }

  std::string vstr = fmt::format("{} (v{}): delta-complete SMT solver ({} Build) {}", DELPI_PROGRAM_NAME, version(),
                                 build_type, repo_stat);
  if (!qsoptex_hash_.empty()) vstr += fmt::format(" (qsopt-ex: {})", qsoptex_hash_);
  if (!soplex_hash_.empty()) vstr += fmt::format(" (soplex: {})", soplex_hash_);
  return vstr;
}

std::ostream &operator<<(std::ostream &os, const ArgParser &parser) { return os << parser.parser_ << std::endl; }

}  // namespace delpi
