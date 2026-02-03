/**
 *
 *  @file scheduler.hpp
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
#ifndef VIX_ASYNC_SCHEDULER_HPP
#define VIX_ASYNC_SCHEDULER_HPP

#include <coroutine>
#include <cstdint>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <utility>

namespace vix::async::core
{
  /**
   * @brief Minimal single-thread scheduler for tasks and coroutine resumption.
   *
   * scheduler provides a thread-safe queue of jobs and an event loop (run())
   * that executes enqueued work on the calling thread.
   *
   * Supported work items:
   * - Generic callables posted via post(Fn)
   * - Coroutine continuations posted via post(coroutine_handle)
   * - An awaitable (schedule()) to hop onto the scheduler thread from a coroutine
   *
   * This is a small building block designed to be embedded into higher-level
   * runtime contexts (e.g. io_context).
   */
  class scheduler
  {
  public:
    /**
     * @brief Construct a scheduler.
     */
    scheduler() = default;

    /**
     * @brief scheduler is non-copyable.
     */
    scheduler(const scheduler &) = delete;

    /**
     * @brief scheduler is non-copyable.
     */
    scheduler &operator=(const scheduler &) = delete;

    /**
     * @brief Post a callable to be executed by the scheduler loop.
     *
     * The callable is enqueued and one waiting thread (run()) is notified.
     *
     * @tparam Fn Callable type.
     * @param fn Callable to enqueue.
     */
    template <typename Fn>
    void post(Fn &&fn)
    {
      {
        std::lock_guard<std::mutex> lock(m_);
        q_.emplace_back(job{std::forward<Fn>(fn)});
      }
      cv_.notify_one();
    }

    /**
     * @brief Post a coroutine continuation to be resumed by the scheduler.
     *
     * The handle is wrapped into a small job that resumes the coroutine
     * when executed by run().
     *
     * @param h Coroutine handle to resume.
     */
    void post(std::coroutine_handle<> h)
    {
      post([h]() mutable
           { if (h) h.resume(); });
    }

    /**
     * @brief Awaitable used to hop onto the scheduler thread.
     *
     * Typical usage inside a coroutine:
     * @code
     * co_await sched.schedule();
     * @endcode
     *
     * When awaited, the continuation is posted to the scheduler so that
     * it resumes on the scheduler's run() thread.
     */
    struct schedule_awaitable
    {
      /**
       * @brief Target scheduler.
       */
      scheduler *s{};

      /**
       * @brief Always suspend to ensure the continuation is enqueued.
       *
       * @return false (always suspends).
       */
      bool await_ready() const noexcept { return false; }

      /**
       * @brief Enqueue the coroutine continuation.
       *
       * If the scheduler pointer is null, the coroutine is resumed inline
       * as a safe fallback.
       *
       * @param h Coroutine handle to resume.
       */
      void await_suspend(std::coroutine_handle<> h) const noexcept
      {
        if (!s)
        {
          if (h)
            h.resume();
          return;
        }
        s->post(h);
      }

      /**
       * @brief Resume point after scheduling.
       */
      void await_resume() const noexcept {}
    };

    /**
     * @brief Create an awaitable that schedules the awaiting coroutine on this scheduler.
     *
     * @return schedule_awaitable bound to this scheduler.
     */
    schedule_awaitable schedule() noexcept { return schedule_awaitable{this}; }

    /**
     * @brief Run the scheduler event loop on the current thread.
     *
     * This function blocks, waiting for new jobs. It executes jobs in FIFO
     * order until stop() is requested and the queue is drained.
     */
    void run()
    {
      running_ = true;

      while (true)
      {
        job j;

        {
          std::unique_lock<std::mutex> lock(m_);
          cv_.wait(lock, [&]()
                   { return stop_requested_ || !q_.empty(); });

          if (!q_.empty())
          {
            j = std::move(q_.front());
            q_.pop_front();
          }
          else if (stop_requested_)
          {
            break;
          }
        }

        if (j.fn)
          j.fn();
      }

      running_ = false;
    }

    /**
     * @brief Request the scheduler loop to stop.
     *
     * Wakes all waiters so that run() can observe the stop request.
     * Pending jobs may still execute depending on the loop state.
     */
    void stop() noexcept
    {
      {
        std::lock_guard<std::mutex> lock(m_);
        stop_requested_ = true;
      }
      cv_.notify_all();
    }

    /**
     * @brief Check whether the scheduler loop is currently running.
     *
     * @return true if run() is active, false otherwise.
     */
    bool is_running() const noexcept { return running_; }

    /**
     * @brief Return the number of pending jobs currently in the queue.
     *
     * @return Queue size.
     */
    std::size_t pending() const
    {
      std::lock_guard<std::mutex> lock(m_);
      return q_.size();
    }

  private:
    /**
     * @brief Type-erased job stored in the scheduler queue.
     *
     * job contains a small type-erased callable holder (fn_holder) that
     * owns the posted callable and invokes it when executed.
     */
    struct job
    {
      /**
       * @brief Default construct an empty job.
       */
      job() = default;

      /**
       * @brief Construct a job from a callable.
       *
       * @tparam Fn Callable type.
       * @param f Callable to store.
       */
      template <typename Fn>
      explicit job(Fn &&f) : fn(std::forward<Fn>(f)) {}

      /**
       * @brief Polymorphic base for type-erased callables.
       */
      struct fn_base
      {
        virtual ~fn_base() = default;

        /**
         * @brief Invoke the stored callable.
         */
        virtual void call() = 0;
      };

      /**
       * @brief Concrete callable wrapper.
       *
       * @tparam Fn Stored callable type.
       */
      template <typename Fn>
      struct fn_impl final : fn_base
      {
        /**
         * @brief Stored callable.
         */
        Fn f;

        /**
         * @brief Construct from a callable instance.
         *
         * @param x Callable to store.
         */
        explicit fn_impl(Fn x) : f(std::move(x)) {}

        /**
         * @brief Invoke the callable.
         */
        void call() override { f(); }
      };

      /**
       * @brief Owning holder for a type-erased callable.
       *
       * This holder manages the lifetime of the polymorphic callable and
       * supports move-only semantics to keep job movable.
       */
      struct fn_holder
      {
        /**
         * @brief Construct an empty holder.
         */
        fn_holder() = default;

        /**
         * @brief Construct a holder from a callable.
         *
         * The callable is stored by value after decay.
         *
         * @tparam Fn Callable type.
         * @param f Callable to store.
         */
        template <typename Fn>
        explicit fn_holder(Fn &&f)
        {
          using D = std::decay_t<Fn>;
          ptr = new fn_impl<D>(D(std::forward<Fn>(f)));
        }

        /**
         * @brief Move construct the holder.
         *
         * @param o Source holder.
         */
        fn_holder(fn_holder &&o) noexcept : ptr(o.ptr) { o.ptr = nullptr; }

        /**
         * @brief Move assign the holder.
         *
         * @param o Source holder.
         * @return Reference to this.
         */
        fn_holder &operator=(fn_holder &&o) noexcept
        {
          if (this != &o)
          {
            reset();
            ptr = o.ptr;
            o.ptr = nullptr;
          }
          return *this;
        }

        /**
         * @brief fn_holder is non-copyable.
         */
        fn_holder(const fn_holder &) = delete;

        /**
         * @brief fn_holder is non-copyable.
         */
        fn_holder &operator=(const fn_holder &) = delete;

        /**
         * @brief Destroy the holder and release any stored callable.
         */
        ~fn_holder() { reset(); }

        /**
         * @brief Invoke the stored callable (if any).
         */
        void operator()()
        {
          if (ptr)
            ptr->call();
        }

        /**
         * @brief Check whether a callable is stored.
         *
         * @return true if non-empty, false otherwise.
         */
        explicit operator bool() const noexcept { return ptr != nullptr; }

        /**
         * @brief Release the stored callable.
         */
        void reset() noexcept
        {
          delete ptr;
          ptr = nullptr;
        }

        /**
         * @brief Pointer to the polymorphic callable.
         */
        fn_base *ptr{nullptr};
      };

      /**
       * @brief Stored callable holder.
       */
      fn_holder fn{};
    };

  private:
    /**
     * @brief Mutex protecting the job queue and stop flag.
     */
    mutable std::mutex m_;

    /**
     * @brief Condition variable used to wake run() when jobs arrive or stop is requested.
     */
    std::condition_variable cv_;

    /**
     * @brief FIFO job queue.
     */
    std::deque<job> q_;

    /**
     * @brief Stop request flag observed by run().
     */
    bool stop_requested_{false};

    /**
     * @brief Indicates whether run() is currently active.
     */
    bool running_{false};
  };

} // namespace vix::async::core

#endif // VIX_ASYNC_SCHEDULER_HPP
