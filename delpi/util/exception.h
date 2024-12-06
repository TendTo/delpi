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

class DelpiException : public std::runtime_error {
 public:
  explicit DelpiException(const char* const message) : std::runtime_error{message} {}
  explicit DelpiException(const std::string& message) : std::runtime_error{message} {}
};

class DelpiNotImplementedException : public DelpiException {
 public:
  DelpiNotImplementedException() : DelpiException("Not yet implemented") {}
  explicit DelpiNotImplementedException(const char* const message) : DelpiException{message} {}
  explicit DelpiNotImplementedException(const std::string& message) : DelpiException{message} {}
};

class DelpiNotSupportedException : public DelpiException {
 public:
  DelpiNotSupportedException() : DelpiException("Not supported") {}
  explicit DelpiNotSupportedException(const char* const message) : DelpiException{message} {}
  explicit DelpiNotSupportedException(const std::string& message) : DelpiException{message} {}
};

class DelpiInvalidArgumentException : public DelpiException, private std::invalid_argument {
 public:
  DelpiInvalidArgumentException() : DelpiException("Invalid argument"), std::invalid_argument{""} {}
  explicit DelpiInvalidArgumentException(const char* const message)
      : DelpiException{message}, std::invalid_argument{message} {}
  explicit DelpiInvalidArgumentException(const std::string& message)
      : DelpiException{message}, std::invalid_argument{message} {}
};

class DelpiAssertionException : public DelpiException {
 public:
  explicit DelpiAssertionException(const char* const message) : DelpiException{message} {}
  explicit DelpiAssertionException(const std::string& message) : DelpiException{message} {}
};

class DelpiLpSolverException : public DelpiException {
 public:
  explicit DelpiLpSolverException(const char* const message) : DelpiException{message} {}
  explicit DelpiLpSolverException(const std::string& message) : DelpiException{message} {}
};

class DelpiSatSolverException : public DelpiException {
 public:
  explicit DelpiSatSolverException(const char* const message) : DelpiException{message} {}
  explicit DelpiSatSolverException(const std::string& message) : DelpiException{message} {}
};

class DelpiParserException : public DelpiException {
 public:
  explicit DelpiParserException(const char* const message) : DelpiException{message} {}
  explicit DelpiParserException(const std::string& message) : DelpiException{message} {}
};

class DelpiOutOfRangeException : public DelpiException, private std::out_of_range {
 public:
  explicit DelpiOutOfRangeException(const char* const message) : DelpiException{message}, std::out_of_range{message} {}
  explicit DelpiOutOfRangeException(const std::string& message) : DelpiException{message}, std::out_of_range{message} {}
};

class DelpiUnreachableException : public DelpiException {
 public:
  DelpiUnreachableException() : DelpiException("Unreachable code") {}
  explicit DelpiUnreachableException(const char* const message) : DelpiException{message} {}
  explicit DelpiUnreachableException(const std::string& message) : DelpiException{message} {}
};

}  // namespace delpi
