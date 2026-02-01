#pragma once

#include <type_traits>
#include <utility>

namespace cnerium::detail
{

  // ================================
  // Type traits shortcuts
  // ================================

  template <typename T>
  using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

  template <typename T>
  inline constexpr bool is_void_v = std::is_void_v<T>;

  template <typename T>
  inline constexpr bool is_nothrow_move_v = std::is_nothrow_move_constructible_v<T>;

  // ================================
  // Callable detection
  // ================================

  template <typename F, typename... Args>
  using is_invocable = std::is_invocable<F, Args...>;

  template <typename F, typename... Args>
  inline constexpr bool is_invocable_v = is_invocable<F, Args...>::value;

} // namespace cnerium::detail
