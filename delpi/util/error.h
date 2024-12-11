/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Utilities that verify assumptions made by the program and aborts
 * the program if those assumptions are not true.
 *
 * If NDEBUG is defined, most of the macro do nothing and give no explanation.
 * It makes the program faster, but less useful for debugging.
 */
#pragma once

#include <fmt/core.h>

#include <stdexcept>

#include "delpi/util/exception.h"

#ifdef NDEBUG

#define DELPI_ASSERT(condition, msg) ((void)0)
#define DELPI_ASSERT_FMT(condition, msg, ...) ((void)0)
#define DELPI_UNREACHABLE() std::terminate()
#define DELPI_RUNTIME_ERROR(msg) throw ::delpi::DelpiException(msg)
#define DELPI_RUNTIME_ERROR_FMT(msg, ...) throw ::delpi::DelpiException(msg)
#define DELPI_OUT_OF_RANGE(msg) throw ::delpi::DelpiOutOfRangeException(msg)
#define DELPI_OUT_OF_RANGE_FMT(msg, ...) throw ::delpi::DelpiOutOfRangeException(msg)
#define DELPI_INVALID_ARGUMENT(argument, actual) throw ::delpi::DelpiException(argument)
#define DELPI_INVALID_ARGUMENT_EXPECTED(argument, actual, expected) \
  throw ::delpi::DelpiInvalidArgumentException(                     \
      fmt::format("Invalid argument for {}: received '{}', expected '{}'", argument, actual, expected))

#else

#include "delpi/util/logging.h"

#define DELPI_ASSERT(condition, message)                                                                 \
  do {                                                                                                   \
    if (!(condition)) {                                                                                  \
      DELPI_CRITICAL_FMT("Assertion `{}` failed in {}:{}: {}", #condition, __FILE__, __LINE__, message); \
      throw ::delpi::DelpiAssertionException(                                                            \
          fmt::format("Assertion `{}` failed in {}:{}: {}", #condition, __FILE__, __LINE__, message));   \
    }                                                                                                    \
  } while (false)

#define DELPI_ASSERT_FMT(condition, message, ...)                                                                  \
  do {                                                                                                             \
    if (!(condition)) {                                                                                            \
      DELPI_CRITICAL_FMT("Assertion `{}` failed in {}:{}\n" message, #condition, __FILE__, __LINE__, __VA_ARGS__); \
      throw ::delpi::DelpiAssertionException(                                                                      \
          fmt::format("Assertion `{}` failed in {}:{}: " message, #condition, __FILE__, __LINE__, __VA_ARGS__));   \
    }                                                                                                              \
  } while (false)

#define DELPI_UNREACHABLE()                                                                                      \
  do {                                                                                                           \
    DELPI_CRITICAL_FMT("{}:{} Should not be reachable.", __FILE__, __LINE__);                                    \
    throw ::delpi::DelpiUnreachableException(fmt::format("{}:{} Should not be reachable.", __FILE__, __LINE__)); \
  } while (false)

#define DELPI_RUNTIME_ERROR(msg)        \
  do {                                  \
    DELPI_CRITICAL(msg);                \
    throw ::delpi::DelpiException(msg); \
  } while (false)

#define DELPI_RUNTIME_ERROR_FMT(msg, ...)                         \
  do {                                                            \
    DELPI_CRITICAL_FMT(msg, __VA_ARGS__);                         \
    throw ::delpi::DelpiException(fmt::format(msg, __VA_ARGS__)); \
  } while (false)

#define DELPI_OUT_OF_RANGE(msg)                   \
  do {                                            \
    DELPI_CRITICAL(msg);                          \
    throw ::delpi::DelpiOutOfRangeException(msg); \
  } while (false)

#define DELPI_OUT_OF_RANGE_FMT(msg, ...)                                    \
  do {                                                                      \
    DELPI_CRITICAL_FMT(msg, __VA_ARGS__);                                   \
    throw ::delpi::DelpiOutOfRangeException(fmt::format(msg, __VA_ARGS__)); \
  } while (false)

#define DELPI_INVALID_ARGUMENT(argument, actual) \
  throw ::delpi::DelpiInvalidArgumentException(fmt::format("Invalid argument for {}: {}", argument, actual))

#define DELPI_INVALID_ARGUMENT_EXPECTED(argument, actual, expected) \
  throw ::delpi::DelpiInvalidArgumentException(                     \
      fmt::format("Invalid argument for {}: received '{}', expected '{}'", argument, actual, expected))

#endif  // NDEBUG
