/**
 *
 *  @file thread_pool.hpp
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
#ifndef VIX_ASYNC_THREAD_POOL_HPP
#define VIX_ASYNC_THREAD_POOL_HPP

#include <cstddef>
#include <coroutine>
#include <deque>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
#include <optional>
#include <system_error>

#include <vix/async/core/cancel.hpp>
#include <vix/async/core/error.hpp>
#include <vix/async/core/task.hpp>

namespace vix::async::core
{
  class io_context;

  namespace detail
  {
    /**
     * @brief Small result container for thread_pool awaitables.
     *
     * Stores an optional value produced by a compute job and provides
     * a take() method to move it out when resuming on the awaiting coroutine.
     *
     * @tparam R Result type.
     */
    template <typename R>
    struct result_store
    {
      /**
       * @brief Stored value (present once job completed successfully).
       */
      std::optional<R> value{};

      /**
       * @brief Store the result value.
       *
       * @param v Value to store.
       */
      void set(R &&v) { value.emplace(std::move(v)); }

      /**
       * @brief Move the stored value out.
       *
       * @return Stored value (moved).
       */
      R take() { return std::move(*value); }
    };

    /**
     * @brief Specialization for void results.
     */
    template <>
    struct result_store<void>
    {
      /**
       * @brief Store completion for a void job.
       */
      void set() noexcept {}
    };
  } // namespace detail

  /**
   * @brief Simple CPU thread pool integrated with io_context.
   *
   * thread_pool runs submitted work on worker threads and resumes awaiting
   * coroutines back onto the io_context scheduler thread.
   *
   * The pool supports:
   * - fire-and-forget submission via submit(std::function<void()>)
   * - coroutine-friendly submission via submit(Fn, cancel_token) returning task<R>
   *
   * Cancellation:
   * - If the provided cancel_token is already cancelled when the worker starts,
   *   the job fails with a cancellation error (std::system_error(cancelled_ec())).
   *
   * Exceptions:
   * - Exceptions thrown by the job are captured and rethrown on await_resume().
   */
  class thread_pool
  {
  public:
    /**
     * @brief Construct a thread pool.
     *
     * @param ctx Runtime context used to post coroutine resumptions.
     * @param threads Number of worker threads (defaults to hardware concurrency).
     */
    explicit thread_pool(
        io_context &ctx,
        std::size_t threads = std::thread::hardware_concurrency());

    /**
     * @brief Destroy the thread pool.
     *
     * Stops workers and joins all threads.
     */
    ~thread_pool();

    /**
     * @brief thread_pool is non-copyable.
     */
    thread_pool(const thread_pool &) = delete;

    /**
     * @brief thread_pool is non-copyable.
     */
    thread_pool &operator=(const thread_pool &) = delete;

    /**
     * @brief Submit a fire-and-forget job to the pool.
     *
     * @param fn Job to execute.
     */
    void submit(std::function<void()> fn);

    /**
     * @brief Submit a job to the pool and co_await its result.
     *
     * This overload wraps the job into an awaitable that:
     * - enqueues the job on the worker queue
     * - runs the job on a worker thread
     * - captures the result or exception
     * - posts the awaiting coroutine back onto the io_context scheduler
     *
     * @tparam Fn Callable type.
     * @param fn Callable to execute on the pool.
     * @param ct Cancellation token.
     * @return task<R> where R is std::invoke_result_t<Fn>.
     *
     * @throws std::system_error with cancelled_ec() when cancellation is observed.
     * @throws Any exception thrown by the callable (re-thrown on await_resume()).
     */
    template <typename Fn>
    auto submit(Fn &&fn, cancel_token ct = {}) -> task<std::invoke_result_t<Fn>>
    {
      using R = std::invoke_result_t<Fn>;

      /**
       * @brief Awaitable implementing the pool submission behavior.
       */
      struct awaitable
      {
        /**
         * @brief Target pool.
         */
        thread_pool *pool{};

        /**
         * @brief Cancellation token.
         */
        cancel_token ct{};

        /**
         * @brief Stored callable (decayed).
         */
        std::decay_t<Fn> fn;

        /**
         * @brief Storage for result (or completion for void).
         */
        detail::result_store<R> store{};

        /**
         * @brief Captured exception from worker execution.
         */
        std::exception_ptr ex{};

        /**
         * @brief Always suspend to run on a worker thread.
         */
        bool await_ready() const noexcept { return false; }

        /**
         * @brief Enqueue the job and suspend the awaiting coroutine.
         *
         * The job executes on a worker thread and then posts the awaiting
         * coroutine handle back onto the io_context.
         *
         * @param h Awaiting coroutine handle.
         */
        void await_suspend(std::coroutine_handle<> h)
        {
          pool->enqueue([this, h]() mutable
                        {
        try
        {
          if (ct.is_cancelled())
            throw std::system_error(cancelled_ec());

          if constexpr (std::is_void_v<R>)
          {
            fn();
            store.set();
          }
          else
          {
            R r = fn();
            store.set(std::move(r));
          }
        }
        catch (...)
        {
          ex = std::current_exception();
        }

        pool->ctx_post(h); });
        }

        /**
         * @brief Resume on the scheduler thread and return the result.
         *
         * Rethrows any exception captured in the worker thread.
         *
         * @return For non-void jobs: computed result (moved). For void: returns void.
         * @throws Any exception thrown by the job (re-thrown here).
         */
        R await_resume()
        {
          if (ex)
            std::rethrow_exception(ex);

          if constexpr (std::is_void_v<R>)
          {
            return;
          }
          else
          {
            return store.take();
          }
        }
      };

      co_return co_await awaitable{this, std::move(ct), std::forward<Fn>(fn)};
    }

    /**
     * @brief Request the pool to stop.
     *
     * Signals worker threads to exit. Pending jobs may still run depending
     * on implementation of worker_loop().
     */
    void stop() noexcept;

    /**
     * @brief Number of worker threads in the pool.
     *
     * @return Worker count.
     */
    std::size_t size() const noexcept { return workers_.size(); }

  private:
    /**
     * @brief Worker thread loop that consumes queued jobs.
     */
    void worker_loop();

    /**
     * @brief Enqueue a job into the worker queue.
     *
     * @param fn Job to enqueue.
     */
    void enqueue(std::function<void()> fn);

    /**
     * @brief Post a coroutine continuation back onto the io_context scheduler.
     *
     * @param h Coroutine handle to resume.
     */
    void ctx_post(std::coroutine_handle<> h);

  private:
    /**
     * @brief Bound runtime context.
     */
    io_context &ctx_;

    /**
     * @brief Mutex protecting job queue and stop flag.
     */
    mutable std::mutex m_;

    /**
     * @brief Condition variable to wake workers.
     */
    std::condition_variable cv_;

    /**
     * @brief FIFO job queue executed by workers.
     */
    std::deque<std::function<void()>> q_;

    /**
     * @brief Stop request flag observed by worker threads.
     */
    bool stop_{false};

    /**
     * @brief Worker threads.
     */
    std::vector<std::thread> workers_;
  };

} // namespace vix::async::core

#endif // VIX_ASYNC_THREAD_POOL_HPP
