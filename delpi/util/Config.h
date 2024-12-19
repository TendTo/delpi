/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Config class.
 * Used to store the configuration of the program.
 * Simple dataclass used to store the configuration of the program.
 * It is generated from @ref delpi::ArgParser.
 */
#pragma once

#include <iosfwd>
#include <string>
#include <string_view>

#include "delpi/util/OptionValue.hpp"

namespace delpi {

#define DELPI_PARAMETER(param_name, type, default_value, help)                             \
 public:                                                                                   \
  /** @getter{`##param_name##` parameter, configuration, Default to default_value##} */    \
  OptionValue<type> &m_##param_name() { return param_name##_; }                            \
  /** @getsetter{`##param_name##` parameter, configuration, Default to default_value##} */ \
  [[nodiscard]] const type &param_name() const { return param_name##_.get(); }             \
  static constexpr type default_##param_name{default_value};                               \
  static constexpr const char *const help_##param_name{help};                              \
                                                                                           \
 private:                                                                                  \
  OptionValue<type> param_name##_{default_value};

/**
 * Simple dataclass used to store the configuration of the program.
 * It is usually generated by the @ref ArgParser using the command line arguments.
 */
class Config {
 public:
  /** Underlying LP solver used by the theory solver. */
  enum class LpSolver {
    SOPLEX,   ///< Soplex Solver. Default option
    QSOPTEX,  ///< Qsoptex Solver
  };
  /** Format of the input file. */
  enum class Format {
    AUTO,  ///< Automatically detect the input format based on the file extension. Default option
    MPS,   ///< MPS format
  };
  /** LP mode used by the LP solver. */
  enum class LpMode {
    AUTO = 0,                       ///< Let the LP solver choose the mode. Default option
    PURE_PRECISION_BOOSTING = 1,    ///< Use the precision boosting mode, if available
    PURE_ITERATIVE_REFINEMENT = 2,  ///< Use the iterative refinement mode, if available
    HYBRID = 3,                     ///< Use both modes, if available
  };

  /** @constructor{Config} */
  Config() = default;
  /**
   * Construct a new Config object with the given filename.
   * @param filename name of the input file
   */
  explicit Config(std::string filename);
  /**
   * Construct a new Config object that will read the input from the standard input.
   * @param read_from_stdin whether to read the input from the standard input
   */
  explicit Config(bool read_from_stdin);
  /**
   * Construct a new Config object and immediately set the format.
   * @param format whether to read the input from the standard input
   */
  explicit Config(Format format);

 public:
  static constexpr std::string_view help_filename{"Input file name"};

  /** @getter{`filename` parameter, configuration, Default to ""}*/
  [[nodiscard]] const std::string &filename() const { return filename_.get(); }
  /** @getter{`filename` extension, configuration, Contains the @ref filename substring after the dot.}*/
  [[nodiscard]] std::string filename_extension() const;
  /** @getsetter{`filename` extension, configuration, Contains the @ref filename substring after the dot.}*/
  OptionValue<std::string> &m_filename() { return filename_; }
  /**
   * @getter{actual `lp_mode` parameter, configuration,
     If the lp_mode is LPMode::AUTO\, it will return the appropriate mode based on the lp_solver}
   */
  [[nodiscard]] LpMode actual_lp_mode() const;
  /**
   * @getter{actual `format` parameter, configuration,
     If the format is Format::AUTO\, it will return the appropriate format based on the filename extension}
   */
  [[nodiscard]] Format actual_format() const;

 private:
  OptionValue<std::string> filename_{""};

  DELPI_PARAMETER(continuous_output, bool, false, "Continuous output")
  DELPI_PARAMETER(csv, bool, false, "Produce CSV output. Must also specify --with-timings to get the time stats")
  DELPI_PARAMETER(debug_parsing, bool, false, "Debug parsing")
  DELPI_PARAMETER(debug_scanning, bool, false, "Debug scanning/lexing")
  DELPI_PARAMETER(format, Format, delpi::Config::Format::AUTO,
                  "Input file format\n"
                  "\t\tOne of: auto (1), mps (2)")
  DELPI_PARAMETER(lp_mode, LpMode, delpi::Config::LpMode::AUTO,
                  "LP mode used by the LP solver.\n"
                  "\t\tOne of: auto (1), pure-precision-boosting (2), pure-iterative-refinement (3), hybrid (4)")
  DELPI_PARAMETER(lp_solver, LpSolver, delpi::Config::LpSolver::SOPLEX,
                  "Underlying LP solver used by the theory solver.\n"
                  "\t\tOne of: soplex (1), qsoptex (2)")
  DELPI_PARAMETER(number_of_jobs, unsigned int, 1u, "Number of jobs")
  DELPI_PARAMETER(skip_optimise, bool, false,
                  "Whether to skip the objective function, turning the optimisation in a feasibility problem. "
                  "Only affects the MPS format")
  DELPI_PARAMETER(precision, double, 9.999999999999996e-4,
                  "Delta precision used by the LP solver solver.\n"
                  "\t\tEven when set to 0, a positive infinitesimal value will be considered.\n"
                  "\t\tWhile the LP solver will yield an exact solution, strict inequalities will still be relaxed\n"
                  "\t\tUse the --complete flag if you are looking for a complete solution")
  DELPI_PARAMETER(produce_models, bool, false,
                  "Produce models, showing a valid assignment.\n"
                  "\t\tOnly applicable if the result is sat or delta-sat")
  DELPI_PARAMETER(random_seed, unsigned int, 0u,
                  "Set the random seed. 0 means that the seed will be generated on the fly")
  DELPI_PARAMETER(read_from_stdin, bool, false, "Read the input from the standard input")
  DELPI_PARAMETER(silent, bool, false, "Silent mode. Nothing will be printed on the standard output")
  DELPI_PARAMETER(
      timeout, unsigned int, 0,
      "Timeout in milliseconds for the main routine, without accounting for input parsing. 0 means no timeout")
  DELPI_PARAMETER(verbose_delpi, int, 2, "Verbosity level for delpi. In the range [0, 5]")
  DELPI_PARAMETER(verbose_simplex, int, 0, "Verbosity level for simplex. In the range [0, 5]")
  DELPI_PARAMETER(verify, bool, false, "If the input produces a SAT output, verify the assignment against the input")
  DELPI_PARAMETER(with_timings, bool, false, "Report timings alongside results")
};

std::ostream &operator<<(std::ostream &os, const Config &config);
std::ostream &operator<<(std::ostream &os, const Config::LpSolver &lp_solver);
std::ostream &operator<<(std::ostream &os, const Config::Format &format);
std::ostream &operator<<(std::ostream &os, const Config::LpMode &mode);

}  // namespace delpi

#ifdef DELPI_INCLUDE_FMT

#include "delpi/util/logging.h"

OSTREAM_FORMATTER(delpi::Config);
OSTREAM_FORMATTER(delpi::Config::LpSolver);
OSTREAM_FORMATTER(delpi::Config::Format);
OSTREAM_FORMATTER(delpi::Config::LpMode);

#endif
