/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * ArgParser class.
 */
#pragma once

#include <iosfwd>
#include <string>

// Argparse library is used to parse the command line arguments.
#include <argparse/argparse.hpp>

#include "delpi/util/Config.h"

namespace delpi {

/**
 * Used to parse command line arguments and produce a corresponding Config object to be used throughout the execution.
 * The default values of each parameter are defined in the Config class.
 * Besides parsing, a validation step is performed to ensure the correctness of the parameters.
 * If an inconsistency is found, an explication of the error is printed to the console.
 * Under the hood, it uses the argparse library.
 */
class ArgParser {
 public:
  /** @constructor{argparser} */
  ArgParser();
  /**
   * Parse the command line arguments.
   * @param argc number of arguments
   * @param argv array of arguments as strings
   */
  void Parse(int argc, const char **argv);

  /** @getter{version, program} */
  [[nodiscard]] static std::string version();
  /** @getter{hash status, repository} */
  [[nodiscard]] static std::string repository_status();
  /** @getter{printable console prompt, program} */
  [[nodiscard]] std::string prompt() const;

  /**
   * Convert the parser to a @ref Config.
   *
   * The @ref Config object will be used throughout the program to pass the configuration to all the components.
   * @return configuration object
   */
  [[nodiscard]] Config ToConfig() const;

  /**
   * Get the value of a parameter from the internal parser.
   * @tparam T type of the parameter
   * @param key name of the parameter
   * @return value of the parameter
   * @throw std::invalid_argument if the key is not found
   */
  template <class T>
  [[nodiscard]] T get(const std::string &key) const {
    return parser_.get<T>(key);
  }

 private:
  friend std::ostream &operator<<(std::ostream &os, const ArgParser &parser);

  /** Add all the options, positional arguments and flags to the parser. */
  void AddOptions();

  /**
   * Validate the options, ensuring the correctness of the parameters and the consistency of the options.
   * @throw DelpiInvalidArgumentException if the options are inconsistent or incorrect.
   * @throw DelpiException if an error occurs during parsing.
   */
  void ValidateOptions();

  argparse::ArgumentParser parser_;  ///< The parser object.
  const std::string qsoptex_hash_;   ///< The hash of the QSoptEx library. Used in the prompt
  const std::string soplex_hash_;    ///< The hash of the Soplex library. Used in the prompt
  int verbosity_;                    ///< Verbosity level of the program
};

}  // namespace delpi
