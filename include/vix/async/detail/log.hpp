/**
 *
 *  @file log.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/vixcpp/vix
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 *
 */
#ifndef VIX_ASYNC_LOG_HPP
#define VIX_ASYNC_LOG_HPP

#include <atomic>
#include <chrono>
#include <ctime>
#include <iostream>
#include <mutex>
#include <string_view>

namespace vix::async::detail
{
  /**
   * @brief Logging severity levels for the async runtime.
   *
   * Levels are ordered from most verbose (trace) to completely disabled (off).
   * Messages with a level lower than the current global level are ignored.
   */
  enum class log_level : int
  {
    trace = 0, /**< Very verbose diagnostic output */
    debug,     /**< Debug-level information */
    info,      /**< Informational messages (default) */
    warn,      /**< Warnings indicating potential issues */
    error,     /**< Errors that occurred but are recoverable */
    fatal,     /**< Fatal errors, aborts the process */
    off        /**< Disable all logging */
  };

  /**
   * @brief Global logging level.
   *
   * Controls the minimum severity level that will be emitted.
   * Defaults to log_level::info.
   */
  inline std::atomic<log_level> g_log_level{log_level::info};

  /**
   * @brief Global mutex protecting log output.
   *
   * Ensures log lines are emitted atomically and never interleaved
   * across threads.
   */
  inline std::mutex g_log_mutex;

  /**
   * @brief Convert a log level to its textual representation.
   *
   * @param lvl Log level.
   * @return Null-terminated string representing the level.
   */
  inline const char *to_string(log_level lvl)
  {
    switch (lvl)
    {
    case log_level::trace:
      return "Trace";
    case log_level::debug:
      return "Debug";
    case log_level::info:
      return "Info";
    case log_level::warn:
      return "Warn";
    case log_level::error:
      return "Error";
    case log_level::fatal:
      return "Fatal";
    default:
      return "Of";
    }
  }

  /**
   * @brief Set the global log level.
   *
   * Messages below this level will be filtered out.
   *
   * @param lvl New global log level.
   */
  inline void set_log_level(log_level lvl) noexcept
  {
    g_log_level.store(lvl, std::memory_order_relaxed);
  }

  /**
   * @brief Get the current global log level.
   *
   * @return Current log level.
   */
  inline log_level get_log_level() noexcept
  {
    return g_log_level.load(std::memory_order_relaxed);
  }

  /**
   * @brief Emit a log message.
   *
   * This function:
   * - checks the global log level
   * - serializes output using a mutex
   * - prepends a local timestamp and severity tag
   * - writes to stderr
   * - aborts the process if the level is fatal
   *
   * @param lvl Severity level of the message.
   * @param msg Message text.
   */
  inline void log(log_level lvl, std::string_view msg)
  {
    if (lvl < get_log_level())
      return;

    std::lock_guard<std::mutex> lock(g_log_mutex);

    // Timestamp (HH:MM:SS, local time)
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    char buf[32];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);

    std::cerr << "[" << buf << "] "
              << "[" << to_string(lvl) << "] "
              << msg << "\n";

    if (lvl == log_level::fatal)
      std::abort();
  }

  /**
   * @def ASYNC_LOG_TRACE(msg)
   * @brief Emit a TRACE-level log message.
   */
#define ASYNC_LOG_TRACE(msg) ::vix::async::detail::log(::vix::async::detail::log_level::trace, msg)

  /**
   * @def ASYNC_LOG_DEBUG(msg)
   * @brief Emit a DEBUG-level log message.
   */
#define ASYNC_LOG_DEBUG(msg) ::vix::async::detail::log(::vix::async::detail::log_level::debug, msg)

  /**
   * @def ASYNC_LOG_INFO(msg)
   * @brief Emit an INFO-level log message.
   */
#define ASYNC_LOG_INFO(msg) ::vix::async::detail::log(::vix::async::detail::log_level::info, msg)

  /**
   * @def ASYNC_LOG_WARN(msg)
   * @brief Emit a WARN-level log message.
   */
#define ASYNC_LOG_WARN(msg) ::vix::async::detail::log(::vix::async::detail::log_level::warn, msg)

  /**
   * @def ASYNC_LOG_ERROR(msg)
   * @brief Emit an ERROR-level log message.
   */
#define ASYNC_LOG_ERROR(msg) ::vix::async::detail::log(::vix::async::detail::log_level::error, msg)

  /**
   * @def ASYNC_LOG_FATAL(msg)
   * @brief Emit a FATAL-level log message and abort the process.
   */
#define ASYNC_LOG_FATAL(msg) ::vix::async::detail::log(::vix::async::detail::log_level::fatal, msg)

} // namespace vix::async::detail

#endif // VIX_ASYNC_LOG_HPP
