#include <iostream>

#include <cnerium/core/io_context.hpp>
#include <cnerium/core/signal.hpp>
#include <cnerium/core/task.hpp>

using cnerium::core::io_context;
using cnerium::core::task;

static task<void> app(io_context &ctx)
{
  auto &sig = ctx.signals();

  // Register signals to handle
  sig.add(SIGINT);
  sig.add(SIGTERM);

  std::cout << "[cnerium] waiting for SIGINT/SIGTERM (Ctrl+C)\n";

  // Option A: handler called on event loop
  sig.on_signal([&](int s)
                {
    std::cout << "[cnerium] signal received: " << s << " -> stopping\n";
    ctx.stop(); });

  // Option B: also show coroutine wait (optional, but useful)
  // If you prefer coroutine style only, remove on_signal and just do async_wait().
  int s = co_await sig.async_wait();
  std::cout << "[cnerium] async_wait got signal: " << s << " -> stopping\n";
  ctx.stop();

  co_return;
}

int main()
{
  io_context ctx;

  auto t = app(ctx);
  ctx.post(t.handle());

  ctx.run();

  std::cout << "[cnerium] stopped\n";
  return 0;
}
