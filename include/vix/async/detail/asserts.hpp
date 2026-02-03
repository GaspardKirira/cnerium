/**
 *
 *  @file asserts.hpp
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
#ifndef VIX_ASYNC_ASSERTS_HPP
#define VIX_ASYNC_ASSERTS_HPP

#include <cstdlib>
#include <iostream>

#include <vix/async/detail/config.hpp>

namespace vix::async::detail
{
  /**
   * @brief Fail-fast assertion handler.
   *
   * Prints a standardized assertion failure message to stderr and aborts.
   * This function is used by ASYNC_ASSERT / ASYNC_ASSERT_MSG when
   * ASYNC_ENABLE_ASSERTS is enabled.
   *
   * @param expr Failed expression as a string.
   * @param file Source file where the assertion failed.
   * @param line Source line where the assertion failed.
   * @param msg Optional custom message (may be null).
   */
  [[noreturn]] inline void assert_fail(
      const char *expr,
      const char *file,
      int line,
      const char *msg = nullptr)
  {
    std::cerr << "[async][assert] failed: " << expr
              << "\n  at " << file << ":" << line;

    if (msg)
      std::cerr << "\n  message: " << msg;

    std::cerr << std::endl;
    std::abort();
  }

} // namespace vix::async::detail

/**
 * @def ASYNC_ASSERT(expr)
 * @brief Public assertion macro for the async module.
 *
 * When ASYNC_ENABLE_ASSERTS is enabled, evaluates @p expr and aborts the
 * program if it is false. When disabled, the macro compiles to a no-op.
 *
 * @param expr Expression to validate.
 */

/**
 * @def ASYNC_ASSERT_MSG(expr, msg)
 * @brief Public assertion macro with a custom message for the async module.
 *
 * When ASYNC_ENABLE_ASSERTS is enabled, evaluates @p expr and aborts the
 * program if it is false, printing the provided message. When disabled,
 * the macro compiles to a no-op.
 *
 * @param expr Expression to validate.
 * @param msg Message to print on failure.
 */

// Public assertion macro
#if ASYNC_ENABLE_ASSERTS
#define ASYNC_ASSERT(expr) \
  ((expr) ? (void)0 : ::vix::async::detail::assert_fail(#expr, __FILE__, __LINE__))

#define ASYNC_ASSERT_MSG(expr, msg) \
  ((expr) ? (void)0 : ::vix::async::detail::assert_fail(#expr, __FILE__, __LINE__, msg))
#else
#define ASYNC_ASSERT(expr) ((void)0)
#define ASYNC_ASSERT_MSG(expr, msg) ((void)0)
#endif

#endif // VIX_ASYNC_ASSERTS_HPP
