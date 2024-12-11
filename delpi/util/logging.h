/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 delpi
 * @licence BSD 3-Clause License
 * Logging macros.
 * Allows logging with different verbosity levels using spdlog.
 *
 * The verbosity level is set with the -V flag.
 * The verbosity level is an integer between 0 and 5 and it increases with each -V flag.
 * It can be reduced with the -q flag.
 * It starts at 2 (warning).
 */
#pragma once

#include <fmt/core.h>  // IWYU pragma: export
#include <fmt/ostream.h>  // IWYU pragma: export
#include <fmt/ranges.h>   // IWYU pragma: export

#ifndef NLOG

#include <spdlog/logger.h>

#include <memory>

namespace delpi {

enum class LoggerType { OUT, ERR };

std::shared_ptr<spdlog::logger> get_logger(LoggerType logger_type);  // NOLINT

}  // namespace delpi

#define OSTREAM_FORMATTER(type) \
  template <>                   \
  struct fmt::formatter<type> : ostream_formatter {};
#define DELPI_FORMAT(message, ...) fmt::format(message, __VA_ARGS__)

#define DELPI_VERBOSITY_TO_LOG_LEVEL(verbosity)                        \
  ((verbosity) == 0                                                    \
       ? spdlog::level::critical                                       \
       : ((verbosity) == 1                                             \
              ? spdlog::level::err                                     \
              : ((verbosity) == 2                                      \
                     ? spdlog::level::warn                             \
                     : ((verbosity) == 3                               \
                            ? spdlog::level::info                      \
                            : ((verbosity) == 4 ? spdlog::level::debug \
                                                : ((verbosity) == 5 ? spdlog::level::trace : spdlog::level::off))))))
#define DELPI_LOG_INIT_VERBOSITY(verbosity) DELPI_LOG_INIT_LEVEL(DELPI_VERBOSITY_TO_LOG_LEVEL(verbosity))
#define DELPI_LOG_INIT_LEVEL(level)                                  \
  do {                                                               \
    ::delpi::get_logger(::delpi::LoggerType::OUT)->set_level(level); \
    ::delpi::get_logger(::delpi::LoggerType::ERR)->set_level(level); \
  } while (0)
#define DELPI_TRACE(msg) ::delpi::get_logger(::delpi::LoggerType::OUT)->trace(msg)
#define DELPI_TRACE_FMT(msg, ...) ::delpi::get_logger(::delpi::LoggerType::OUT)->trace(msg, __VA_ARGS__)
#define DELPI_DEBUG(msg) ::delpi::get_logger(::delpi::LoggerType::OUT)->debug(msg)
#define DELPI_DEBUG_FMT(msg, ...) ::delpi::get_logger(::delpi::LoggerType::OUT)->debug(msg, __VA_ARGS__)
#define DELPI_INFO(msg) ::delpi::get_logger(::delpi::LoggerType::OUT)->info(msg)
#define DELPI_INFO_FMT(msg, ...) ::delpi::get_logger(::delpi::LoggerType::OUT)->info(msg, __VA_ARGS__)
#define DELPI_WARN(msg) ::delpi::get_logger(::delpi::LoggerType::ERR)->warn(msg)
#define DELPI_WARN_FMT(msg, ...) ::delpi::get_logger(::delpi::LoggerType::ERR)->warn(msg, __VA_ARGS__)
#define DELPI_ERROR(msg) ::delpi::get_logger(::delpi::LoggerType::ERR)->error(msg)
#define DELPI_ERROR_FMT(msg, ...) ::delpi::get_logger(::delpi::LoggerType::ERR)->error(msg, __VA_ARGS__)
#define DELPI_CRITICAL(msg) ::delpi::get_logger(::delpi::LoggerType::ERR)->critical(msg)
#define DELPI_CRITICAL_FMT(msg, ...) ::delpi::get_logger(::delpi::LoggerType::ERR)->critical(msg, __VA_ARGS__)
#define DELPI_INFO_ENABLED (::delpi::get_logger(::delpi::LoggerType::OUT)->should_log(spdlog::level::info))
#define DELPI_TRACE_ENABLED (::delpi::get_logger(::delpi::LoggerType::OUT)->should_log(spdlog::level::trace))

#ifndef NDEBUG

#include <thread>

#define DELPI_DEV(msg)                                                                                          \
  do {                                                                                                          \
    if (::delpi::get_logger(::delpi::LoggerType::ERR)->should_log(spdlog::level::err))                          \
      fmt::println("[{:%Y-%m-%d %H:%M:%S}] [\033[1m\033[35mDEV\033[0m] [thread {}] " msg "",                    \
                   std::chrono::system_clock::now(), std::hash<std::thread::id>{}(std::this_thread::get_id())); \
    std::cout << std::flush;                                                                                    \
  } while (0)
#define DELPI_DEV_FMT(msg, ...)                                                                                \
  do {                                                                                                         \
    if (::delpi::get_logger(::delpi::LoggerType::ERR)->should_log(spdlog::level::err))                         \
      fmt::println("[{:%Y-%m-%d %H:%M:%S}] [\033[1m\033[35mDEV\033[0m] [thread {}] " msg "",                   \
                   std::chrono::system_clock::now(), std::hash<std::thread::id>{}(std::this_thread::get_id()), \
                   __VA_ARGS__);                                                                               \
    std::cout << std::flush;                                                                                   \
  } while (0)

#define DELPI_DEV_TRACE(msg) DELPI_DEV(msg)
#define DELPI_DEV_TRACE_FMT(msg, ...) DELPI_DEV_FMT(msg, __VA_ARGS__)
#define DELPI_DEV_DEBUG(msg) DELPI_DEV(msg)
#define DELPI_DEV_DEBUG_FMT(msg, ...) DELPI_DEV_FMT(msg, __VA_ARGS__)
#else
#define DELPI_DEV(msg) void(0)
#define DELPI_DEV_FMT(msg, ...) void(0)
#define DELPI_DEV_TRACE(msg) DELPI_TRACE(msg)
#define DELPI_DEV_TRACE_FMT(msg, ...) DELPI_TRACE_FMT(msg, __VA_ARGS__)
#define DELPI_DEV_DEBUG(msg) DELPI_DEBUG(msg)
#define DELPI_DEV_DEBUG_FMT(msg, ...) DELPI_DEBUG_FMT(msg, __VA_ARGS__)
#endif

#else

#define OSTREAM_FORMATTER(type)
#define DELPI_FORMAT(message, ...) fmt::format(message, __VA_ARGS__)
#define DELPI_VERBOSITY_TO_LOG_LEVEL(verbosity) 0
#define DELPI_LOG_INIT_LEVEL(level) void(0)
#define DELPI_LOG_INIT_VERBOSITY(verbosity) void(0)
#define DELPI_TRACE(msg) void(0)
#define DELPI_TRACE_FMT(msg, ...) void(0)
#define DELPI_DEBUG(msg) void(0)
#define DELPI_DEBUG_FMT(msg, ...) void(0)
#define DELPI_INFO(msg) void(0)
#define DELPI_INFO_FMT(msg, ...) void(0)
#define DELPI_WARN(msg) void(0)
#define DELPI_WARN_FMT(msg, ...) void(0)
#define DELPI_ERROR(msg) void(0)
#define DELPI_ERROR_FMT(msg, ...) void(0)
#define DELPI_CRITICAL(msg) void(0)
#define DELPI_CRITICAL_FMT(msg, ...) void(0)
#define DELPI_INFO_ENABLED false
#define DELPI_TRACE_ENABLED false
#define DELPI_DEV(msg) void(0)
#define DELPI_DEV_FMT(msg, ...) void(0)
#define DELPI_DEV_TRACE(msg) void(0)
#define DELPI_DEV_TRACE_FMT(msg, ...) void(0)
#define DELPI_DEV_DEBUG(msg) void(0)
#define DELPI_DEV_DEBUG_FMT(msg, ...) void(0)

#endif
