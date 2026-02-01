#pragma once

#include <csignal>
#include <coroutine>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

#include <cnerium/core/task.hpp>
#include <cnerium/core/cancel.hpp>
#include <cnerium/core/error.hpp>

namespace cnerium::core
{

  class io_context;

  // signal_set captures OS signals (SIGINT/SIGTERM/...) and delivers them on the event loop.
  //
  // Design v0 (portable enough, safe):
  // - On POSIX: use a dedicated thread + sigwait() to synchronously wait for signals.
  //   When received, post delivery onto io_context scheduler.
  // - On non-POSIX: not supported (returns errc::not_supported).
  //
  // Why not a raw signal handler? Because signal handlers are extremely restricted and unsafe.
  // sigwait() is the clean approach for runtimes.
  class signal_set
  {
  public:
    explicit signal_set(io_context &ctx);
    ~signal_set();

    signal_set(const signal_set &) = delete;
    signal_set &operator=(const signal_set &) = delete;

    // Add a signal to be handled (ex: SIGINT, SIGTERM).
    // Must be called before run() or async_wait() for deterministic behavior.
    void add(int sig);

    // Remove a signal from the set.
    void remove(int sig);

    // Coroutine-friendly wait: completes when one of the registered signals is received.
    // Returns the signal number (ex: SIGINT).
    task<int> async_wait(cancel_token ct = {});

    // Fire-and-forget handler: invoked on the event loop when a signal is received.
    // If you set a handler, it is called for every received signal.
    void on_signal(std::function<void(int)> fn);

    // Stop signal thread and stop delivering signals.
    void stop() noexcept;

  private:
    void start_if_needed();
    void worker_loop();

    void ctx_post(std::function<void()> fn);

  private:
    io_context &ctx_;

    std::mutex m_;

    std::vector<int> signals_;
    std::function<void(int)> on_signal_{};

    // queue of received signals (produced by worker thread, consumed by event loop delivery)
    std::queue<int> pending_;

    bool started_{false};
    bool stop_{false};

    std::thread worker_;

    // Used by async_wait implementation
    std::coroutine_handle<> waiter_{};
    bool waiter_active_{false};
  };

} // namespace cnerium::core
