/**
 *
 *  @file cancel.hpp
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
#ifndef VIX_ASYNC_CANCEL_HPP
#define VIX_ASYNC_CANCEL_HPP

#include <atomic>
#include <memory>

#include <vix/async/core/error.hpp>

namespace vix::async::core
{
  /**
   * @brief Shared cancellation state.
   *
   * cancel_state holds the atomic cancellation flag shared between
   * a cancel_source and all associated cancel_token instances.
   *
   * This object is reference-counted and designed to be safely
   * accessed concurrently from multiple threads.
   */
  class cancel_state
  {
  public:
    /**
     * @brief Request cancellation.
     *
     * Sets the internal cancellation flag. This operation is
     * thread-safe and may be called multiple times.
     */
    void request_cancel() noexcept
    {
      cancelled_.store(true, std::memory_order_release);
    }

    /**
     * @brief Check whether cancellation was requested.
     *
     * @return true if cancellation has been requested, false otherwise.
     */
    bool is_cancelled() const noexcept
    {
      return cancelled_.load(std::memory_order_acquire);
    }

  private:
    /**
     * @brief Atomic cancellation flag.
     */
    std::atomic<bool> cancelled_{false};
  };

  /**
   * @brief Lightweight cancellation observer.
   *
   * cancel_token provides a read-only view of a cancellation state.
   * It does not own the state and cannot request cancellation itself.
   *
   * Tokens are cheap to copy and may be safely passed across threads.
   */
  class cancel_token
  {
  public:
    /**
     * @brief Construct an empty (non-cancelable) token.
     */
    cancel_token() = default;

    /**
     * @brief Construct a token bound to a cancellation state.
     *
     * @param st Shared cancellation state.
     */
    explicit cancel_token(std::shared_ptr<cancel_state> st) noexcept
        : st_(std::move(st)) {}

    /**
     * @brief Check whether this token is associated with a cancel source.
     *
     * @return true if the token can observe cancellation, false otherwise.
     */
    bool can_cancel() const noexcept
    {
      return static_cast<bool>(st_);
    }

    /**
     * @brief Check whether cancellation has been requested.
     *
     * @return true if cancellation was requested, false otherwise.
     */
    bool is_cancelled() const noexcept
    {
      return st_ ? st_->is_cancelled() : false;
    }

  private:
    /**
     * @brief Shared cancellation state.
     */
    std::shared_ptr<cancel_state> st_{};
  };

  /**
   * @brief Cancellation source and owner.
   *
   * cancel_source owns the cancellation state and is responsible
   * for issuing cancellation requests. All tokens produced by
   * this source observe the same cancellation state.
   */
  class cancel_source
  {
  public:
    /**
     * @brief Construct a new cancellation source.
     *
     * The associated cancellation state starts in a non-cancelled state.
     */
    cancel_source()
        : st_(std::make_shared<cancel_state>()) {}

    /**
     * @brief Obtain a cancellation token linked to this source.
     *
     * @return A cancel_token observing this source.
     */
    cancel_token token() const noexcept
    {
      return cancel_token{st_};
    }

    /**
     * @brief Request cancellation.
     *
     * Signals cancellation to all associated tokens.
     */
    void request_cancel() noexcept
    {
      if (st_)
        st_->request_cancel();
    }

    /**
     * @brief Check whether cancellation has been requested.
     *
     * @return true if cancellation was requested, false otherwise.
     */
    bool is_cancelled() const noexcept
    {
      return st_ ? st_->is_cancelled() : false;
    }

  private:
    /**
     * @brief Shared cancellation state.
     */
    std::shared_ptr<cancel_state> st_;
  };

  /**
   * @brief Standard error code for cancellation.
   *
   * @return std::error_code representing a cancelled operation.
   */
  inline std::error_code cancelled_ec() noexcept
  {
    return make_error_code(errc::canceled);
  }

} // namespace vix::async::core

#endif // VIX_ASYNC_CANCEL_HPP
