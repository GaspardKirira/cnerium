/**
 *
 *  @file io_context.hpp
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
#ifndef VIX_ASYNC_IO_CONTEXT_HPP
#define VIX_ASYNC_IO_CONTEXT_HPP

#include <coroutine>
#include <memory>
#include <utility>

#include <vix/async/core/scheduler.hpp>

namespace vix::async::net::detail
{
  /**
   * @brief Internal networking service (Asio-backed).
   *
   * This type is forward-declared to keep io_context lightweight and
   * avoid pulling networking implementation details into the public API.
   */
  class asio_net_service;
}

namespace vix::async::core
{
  class thread_pool;
  class timer;
  class signal_set;

  /**
   * @brief Core runtime context for async operations.
   *
   * io_context owns a scheduler that drives coroutine continuations and
   * posted tasks. It also exposes lazily-created services used by higher
   * level facilities:
   * - CPU thread pool for compute-bound work
   * - timers for scheduling time-based events
   * - signals for signal handling
   * - net for networking (implementation detail service)
   *
   * The context is non-copyable and typically lives for the duration of
   * the program or subsystem using it.
   */
  class io_context
  {
  public:
    /**
     * @brief Construct a new io_context.
     *
     * Initializes the underlying scheduler. Optional services are created
     * lazily on first access through their corresponding accessors.
     */
    io_context();

    /**
     * @brief Destroy the io_context.
     *
     * Destroys any lazily-initialized services and the scheduler.
     */
    ~io_context();

    /**
     * @brief io_context is non-copyable.
     */
    io_context(const io_context &) = delete;

    /**
     * @brief io_context is non-copyable.
     */
    io_context &operator=(const io_context &) = delete;

    /**
     * @brief Access the underlying scheduler.
     *
     * @return Reference to the scheduler.
     */
    scheduler &get_scheduler() noexcept { return sched_; }

    /**
     * @brief Access the underlying scheduler.
     *
     * @return Const reference to the scheduler.
     */
    const scheduler &get_scheduler() const noexcept { return sched_; }

    /**
     * @brief Post a callable to be executed by the scheduler.
     *
     * The callable is forwarded into the scheduler queue and will be
     * executed when run() drives the scheduler.
     *
     * @tparam Fn Callable type.
     * @param fn Callable to enqueue.
     */
    template <typename Fn>
    void post(Fn &&fn)
    {
      sched_.post(std::forward<Fn>(fn));
    }

    /**
     * @brief Post a coroutine continuation to be resumed by the scheduler.
     *
     * @param h Coroutine handle to resume.
     */
    void post(std::coroutine_handle<> h)
    {
      sched_.post(h);
    }

    /**
     * @brief Run the scheduler event loop.
     *
     * This call typically blocks and processes queued tasks until stop()
     * is called or the scheduler decides to return.
     */
    void run()
    {
      sched_.run();
    }

    /**
     * @brief Stop the scheduler.
     *
     * Signals the scheduler to stop processing and return from run().
     */
    void stop() noexcept
    {
      sched_.stop();
    }

    /**
     * @brief Check whether the scheduler is currently running.
     *
     * @return true if running, false otherwise.
     */
    bool is_running() const noexcept
    {
      return sched_.is_running();
    }

    /**
     * @name Lazy services
     *
     * Services below are initialized on first access and are owned by
     * this io_context instance.
     *
     * @{
     */

    /**
     * @brief Access the CPU thread pool service.
     *
     * Lazily constructs the pool on first call.
     *
     * @return Reference to the thread_pool service.
     */
    thread_pool &cpu_pool();

    /**
     * @brief Access the timers service.
     *
     * Lazily constructs the timer service on first call.
     *
     * @return Reference to the timer service.
     */
    timer &timers();

    /**
     * @brief Access the signal handling service.
     *
     * Lazily constructs the signal_set service on first call.
     *
     * @return Reference to the signal_set service.
     */
    signal_set &signals();

    /**
     * @brief Access the networking service.
     *
     * Lazily constructs the internal Asio-backed networking service on
     * first call. The type is intentionally kept in a detail namespace.
     *
     * @return Reference to the asio_net_service.
     */
    vix::async::net::detail::asio_net_service &net();

    /** @} */

  private:
    /**
     * @brief Primary scheduler driving task execution and coroutine resumption.
     */
    scheduler sched_;

    /**
     * @brief Lazily-created CPU pool.
     */
    std::unique_ptr<thread_pool> cpu_pool_;

    /**
     * @brief Lazily-created timers service.
     */
    std::unique_ptr<timer> timer_;

    /**
     * @brief Lazily-created signals service.
     */
    std::unique_ptr<signal_set> signals_;

    /**
     * @brief Lazily-created networking service.
     */
    std::unique_ptr<vix::async::net::detail::asio_net_service> net_;
  };

} // namespace vix::async::core

#endif // VIX_ASYNC_IO_CONTEXT_HPP
