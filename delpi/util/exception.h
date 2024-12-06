/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Exception classes for delpi.
 */
#pragma once

#include <stdexcept>
#include <string>

namespace delpi {

/** Base class for all exceptions in delpi. */
class DelpiException : public std::runtime_error {
 public:
  explicit DelpiException(const char* const message) : std::runtime_error{message} {}
  explicit DelpiException(const std::string& message) : std::runtime_error{message} {}
};

/** Exception for not yet implemented features. */
class DelpiNotImplementedException final : public DelpiException {
 public:
  DelpiNotImplementedException() : DelpiException("Not yet implemented") {}
  explicit DelpiNotImplementedException(const char* const message) : DelpiException{message} {}
  explicit DelpiNotImplementedException(const std::string& message) : DelpiException{message} {}
};

/** Exception for not supported features. */
class DelpiNotSupportedException final : public DelpiException {
 public:
  DelpiNotSupportedException() : DelpiException("Not supported") {}
  explicit DelpiNotSupportedException(const char* const message) : DelpiException{message} {}
  explicit DelpiNotSupportedException(const std::string& message) : DelpiException{message} {}
};

/** Exception for invalid arguments. */
class DelpiInvalidArgumentException final : public DelpiException, private std::invalid_argument {
 public:
  DelpiInvalidArgumentException() : DelpiException("Invalid argument"), std::invalid_argument{""} {}
  explicit DelpiInvalidArgumentException(const char* const message)
      : DelpiException{message}, std::invalid_argument{message} {}
  explicit DelpiInvalidArgumentException(const std::string& message)
      : DelpiException{message}, std::invalid_argument{message} {}
};

/** Exception for assertion failures. */
class DelpiAssertionException final : public DelpiException {
 public:
  explicit DelpiAssertionException(const char* const message) : DelpiException{message} {}
  explicit DelpiAssertionException(const std::string& message) : DelpiException{message} {}
};

/** Exception in the LP solver */
class DelpiLpSolverException final : public DelpiException {
 public:
  explicit DelpiLpSolverException(const char* const message) : DelpiException{message} {}
  explicit DelpiLpSolverException(const std::string& message) : DelpiException{message} {}
};

/** Exception for parser errors. */
class DelpiParserException final : public DelpiException {
 public:
  explicit DelpiParserException(const char* const message) : DelpiException{message} {}
  explicit DelpiParserException(const std::string& message) : DelpiException{message} {}
};

/** Exception for out of range errors. */
class DelpiOutOfRangeException final : public DelpiException, private std::out_of_range {
 public:
  explicit DelpiOutOfRangeException(const char* const message) : DelpiException{message}, std::out_of_range{message} {}
  explicit DelpiOutOfRangeException(const std::string& message) : DelpiException{message}, std::out_of_range{message} {}
};

/** Exception for unreachable code. */
class DelpiUnreachableException final : public DelpiException {
 public:
  DelpiUnreachableException() : DelpiException("Unreachable code") {}
  explicit DelpiUnreachableException(const char* const message) : DelpiException{message} {}
  explicit DelpiUnreachableException(const std::string& message) : DelpiException{message} {}
};

}  // namespace delpi
