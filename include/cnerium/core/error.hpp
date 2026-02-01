#pragma once

#include <cstdint>
#include <string>
#include <system_error>

namespace cnerium::core
{

  // Core error codes for Cnerium.
  // We use std::error_code to integrate naturally with C++ and other libs.
  enum class errc : std::uint8_t
  {
    ok = 0,

    // Generic
    invalid_argument,
    not_ready,
    timeout,
    canceled,
    closed,
    overflow,

    // Scheduler / runtime
    stopped,
    queue_full,

    // Thread pool
    rejected,

    // Signals / timers
    not_supported
  };

  class error_category final : public std::error_category
  {
  public:
    const char *name() const noexcept override
    {
      return "cnerium";
    }

    std::string message(int c) const override
    {
      switch (static_cast<errc>(c))
      {
      case errc::ok:
        return "ok";
      case errc::invalid_argument:
        return "invalid argument";
      case errc::not_ready:
        return "not ready";
      case errc::timeout:
        return "timeout";
      case errc::canceled:
        return "canceled";
      case errc::closed:
        return "closed";
      case errc::overflow:
        return "overflow";
      case errc::stopped:
        return "stopped";
      case errc::queue_full:
        return "queue full";
      case errc::rejected:
        return "rejected";
      case errc::not_supported:
        return "not supported";
      default:
        return "unknown error";
      }
    }
  };

  inline const std::error_category &category() noexcept
  {
    static error_category cat;
    return cat;
  }

  inline std::error_code make_error_code(errc e) noexcept
  {
    return {static_cast<int>(e), category()};
  }

} // namespace cnerium::core

// Enable implicit conversion: errc -> std::error_code
namespace std
{
  template <>
  struct is_error_code_enum<cnerium::core::errc> : true_type
  {
  };
} // namespace std
