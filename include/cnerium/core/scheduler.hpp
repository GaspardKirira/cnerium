#pragma once

#include <coroutine>
#include <cstdint>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <utility>

namespace cnerium::core
{

  // A minimal single-thread event loop (scheduler) that executes posted jobs.
  // It is designed to be the "core loop" of Cnerium.
  // - thread-safe post()
  // - run() must be called by the event-loop thread
  // - stop() wakes run()
  // - supports posting coroutines by scheduling their resumption
  class scheduler
  {
  public:
    scheduler() = default;

    scheduler(const scheduler &) = delete;
    scheduler &operator=(const scheduler &) = delete;

    // Post a simple callable job.
    // The callable is executed on the event-loop thread inside run().
    template <typename Fn>
    void post(Fn &&fn)
    {
      {
        std::lock_guard<std::mutex> lock(m_);
        q_.emplace_back(job{std::forward<Fn>(fn)});
      }
      cv_.notify_one();
    }

    // Schedule the resumption of a coroutine on this scheduler.
    // The handle is resumed on the event-loop thread.
    void post(std::coroutine_handle<> h)
    {
      post([h]() mutable
           { if (h) h.resume(); });
    }

    // Run the loop until stop() is called and the queue is drained.
    // Must be called from the thread that owns the event loop.
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

        // Execute outside the lock.
        if (j.fn)
        {
          j.fn();
        }
      }

      running_ = false;
    }

    // Request graceful stop. run() will exit once the queue is empty.
    void stop() noexcept
    {
      {
        std::lock_guard<std::mutex> lock(m_);
        stop_requested_ = true;
      }
      cv_.notify_all();
    }

    bool is_running() const noexcept { return running_; }

    // Returns how many jobs are pending (approximate; thread-safe).
    std::size_t pending() const
    {
      std::lock_guard<std::mutex> lock(m_);
      return q_.size();
    }

  private:
    struct job
    {
      job() = default;

      template <typename Fn>
      explicit job(Fn &&f) : fn(std::forward<Fn>(f)) {}

      // Small type-erased callable (no std::function allocation by default?).
      // For now we use std::function-like minimal wrapper via std::move-only lambda storage.
      // In next iteration we can replace this with a small-buffer optimization job type.
      struct fn_base
      {
        virtual ~fn_base() = default;
        virtual void call() = 0;
      };

      template <typename Fn>
      struct fn_impl final : fn_base
      {
        Fn f;
        explicit fn_impl(Fn &&x) : f(std::move(x)) {}
        void call() override { f(); }
      };

      struct fn_holder
      {
        fn_holder() = default;

        template <typename Fn>
        explicit fn_holder(Fn &&f)
        {
          ptr = new fn_impl<std::decay_t<Fn>>(std::forward<Fn>(f));
        }

        fn_holder(fn_holder &&o) noexcept : ptr(o.ptr) { o.ptr = nullptr; }
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

        fn_holder(const fn_holder &) = delete;
        fn_holder &operator=(const fn_holder &) = delete;

        ~fn_holder() { reset(); }

        void operator()()
        {
          if (ptr)
            ptr->call();
        }

        explicit operator bool() const noexcept { return ptr != nullptr; }

        void reset() noexcept
        {
          delete ptr;
          ptr = nullptr;
        }

        fn_base *ptr{nullptr};
      };

      fn_holder fn{};
    };

  private:
    mutable std::mutex m_;
    std::condition_variable cv_;
    std::deque<job> q_;

    bool stop_requested_{false};
    bool running_{false};
  };

} // namespace cnerium::core
