/**
 *
 *  @file traits.hpp
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
#ifndef ASYNC_TRAITS_HPP
#define ASYNC_TRAITS_HPP

#include <type_traits>
#include <utility>

namespace vix::async::detail
{
  /**
   * @brief Remove const, volatile, and reference qualifiers from a type.
   *
   * Equivalent to std::remove_cvref_t (C++20), provided here explicitly
   * for internal use and clarity.
   *
   * @tparam T Input type.
   */
  template <typename T>
  using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

  /**
   * @brief Check whether a type is void.
   *
   * Convenience alias around std::is_void_v.
   *
   * @tparam T Type to inspect.
   */
  template <typename T>
  inline constexpr bool is_void_v = std::is_void_v<T>;

  /**
   * @brief Check whether a type is nothrow move constructible.
   *
   * This trait is commonly used to optimize coroutine and task machinery
   * where strong exception guarantees are required.
   *
   * @tparam T Type to inspect.
   */
  template <typename T>
  inline constexpr bool is_nothrow_move_v = std::is_nothrow_move_constructible_v<T>;

  /**
   * @brief Type trait to check if a callable can be invoked with given arguments.
   *
   * Alias for std::is_invocable.
   *
   * @tparam F Callable type.
   * @tparam Args Argument types.
   */
  template <typename F, typename... Args>
  using is_invocable = std::is_invocable<F, Args...>;

  /**
   * @brief Boolean shortcut for is_invocable<F, Args...>.
   *
   * @tparam F Callable type.
   * @tparam Args Argument types.
   */
  template <typename F, typename... Args>
  inline constexpr bool is_invocable_v = is_invocable<F, Args...>::value;

} // namespace vix::async::detail

#endif // ASYNC_TRAITS_HPP
