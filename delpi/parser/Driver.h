/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Driver class.
 */
#pragma once

#include <iosfwd>
#include <string>

#include "delpi/solver/LpSolver.h"
#include "delpi/symbolic/Expression.h"
#include "delpi/util/Stats.h"

namespace delpi {

/**
 * The Driver is the base class for all the parsers.
 *
 * It contains the common logic to allow the parsed data to be saved in the context.
 * It coordinates the communication between the parser (bison) and the scanner (flex).
 */
class Driver {
 public:
  /// construct a new parser driver context
  explicit Driver(LpSolver &lp_solver, const std::string &class_name = "Driver");
  virtual ~Driver() = default;

  /**
   * Invoke the scanner and parser for a stream.
   * @param in input stream
   * @param sname stream name for error messages
   * @return true if successfully parsed
   * @return false if an error occurred
   */
  bool ParseStream(std::istream &in, const std::string &sname = "stream input");

  /**
   * Invoke the scanner and parser on an input string.
   * @param input input string
   * @param sname stream name for error messages
   * @return true if successfully parsed
   * @return false if an error occurred
   */
  bool ParseString(const std::string &input, const std::string &sname = "string stream");

  /**
   * Invoke the scanner and parser on a file.
   *
   * Use parse_stream with a std::ifstream if detection of file reading errors is required.
   * @param filename input file name
   * @return true if successfully parsed
   * @return false if an error occurred
   */
  virtual bool ParseFile(const std::string &filename);

  /** General error handling. */
  static void Error(const std::string &m);

  /** Call context_.CheckSat() and print proper output messages to the standard output. */
  void CheckSat();

  /**
   * @smtcommand{get-assertions, Print all the assertions currently in the context.
     If the mode is set to silent\, it does not print anything.}
   */
  void GetConstraints() const;

  /**
   * @smtcommand{get-info, Print information about the solver or the current context.
     @param key key of the information to print}
   */
  void GetInfo(const std::string &key) const;
  /**
   * @smtcommand{set-info, Set the `value` of the information identified by `key`.
     @param key key of the information to set
     @param value value of the information}
   */
  void SetInfo(const std::string &key, const std::string &value);
  /**
   * @smtcommand{set-option, Set the `value` of the option identified by `key`.
     @param key key of the option to set
     @param value value of the option}
   */
  void SetOption(const std::string &key, const std::string &value);

  /**
   * Maximise the objective function `f`. The objective function is
   * added to the context as a constraint.
   * @param objective_function expression to maximise}
   */
  void Maximise(const Expression &objective_function);

  /**
   * Minimise the objective function `f`. The objective function is
   * added to the context as a constraint.
   * @param objective_function expression to minimise
   */
  void Minimise(const Expression &objective_function);

  /** @smtcommand{exit, Terminate the parsing} */
  void Exit();

  /** @getter{lp solver, Driver} */
  [[nodiscard]] const LpSolver &lp_solver() const { return lp_solver_; }
  /** @getter{config, Driver} */
  [[nodiscard]] const Config &config() const { return lp_solver_.config(); }
  /** @getter{stream name, input being parsed} */
  [[nodiscard]] const std::string &stream_name() const { return stream_name_; }
  /** @getsetter{stream name, input being parsed} */
  std::string &m_stream_name() { return stream_name_; }
  /** @getter{stats, driver} */
  [[nodiscard]] const Stats &stats() const { return stats_; }

 protected:
  /**
   * Parse the stream.
   * @param in input stream
   * @return true if successfully parsed
   * @return false if an error occurred
   */
  virtual bool ParseStreamCore(std::istream &in) = 0;

  std::string stream_name_;  ///< The name of the stream being parsed.

  LpSolver &lp_solver_;  ///< LP parser that will store the parsed data.

  IterationStats stats_;  ///< Statistics for the driver.
};

}  // namespace delpi
