#pragma once

#include <coroutine>
#include <memory>
#include <utility>

#include <cnerium/core/scheduler.hpp>

namespace cnerium::net::detail
{
  class asio_net_service;
}

namespace cnerium::core
{

  class thread_pool;
  class timer;
  class signal_set;

  // io_context is the "runtime container" for Cnerium.
  // It owns (or references) the main scheduler (event loop) and optional subsystems.
  // For now, it provides:
  // - access to the scheduler
  // - run/stop forwarding
  // - posting jobs / coroutine resumption
  //
  // Next headers (thread_pool.hpp, timer.hpp, signal.hpp) will plug into this.
  class io_context
  {
  public:
    io_context() = default;

    io_context(const io_context &) = delete;
    io_context &operator=(const io_context &) = delete;

    // Scheduler access
    scheduler &get_scheduler() noexcept { return sched_; }
    const scheduler &get_scheduler() const noexcept { return sched_; }

    cnerium::net::detail::asio_net_service &net();

    // Convenience helpers
    template <typename Fn>
    void post(Fn &&fn)
    {
      sched_.post(std::forward<Fn>(fn));
    }

    void post(std::coroutine_handle<> h)
    {
      sched_.post(h);
    }

    // Event loop controls
    void run()
    {
      sched_.run();
    }

    void stop() noexcept
    {
      sched_.stop();
    }

    bool is_running() const noexcept
    {
      return sched_.is_running();
    }

    // Subsystems (lazy init)
    thread_pool &cpu_pool(); // defined in thread_pool.cpp (after we add thread_pool)
    timer &timers();         // defined in timer.cpp
    signal_set &signals();   // defined in signal.cpp

  private:
    scheduler sched_;

    // Optional subsystems: created on demand to keep the minimal core light.
    std::unique_ptr<thread_pool> cpu_pool_;
    std::unique_ptr<timer> timer_;
    std::unique_ptr<signal_set> signals_;
    std::unique_ptr<cnerium::net::detail::asio_net_service> net_;
  };

} // namespace cnerium::core
