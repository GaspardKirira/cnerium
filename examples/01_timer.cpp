#include <iostream>

#include <cnerium/core/io_context.hpp>
#include <cnerium/core/task.hpp>
#include <cnerium/core/timer.hpp>

using cnerium::core::io_context;
using cnerium::core::task;

static task<void> app(io_context &ctx)
{
  std::cout << "[cnerium] timer demo start\n";

  co_await ctx.timers().sleep_for(std::chrono::milliseconds(100));
  std::cout << "[cnerium] +100ms\n";

  co_await ctx.timers().sleep_for(std::chrono::milliseconds(200));
  std::cout << "[cnerium] +200ms\n";

  // Fire-and-forget callback after 150ms
  ctx.timers().after(std::chrono::milliseconds(150), [&]()
                     { std::cout << "[cnerium] after(150ms) callback\n"; });

  // Wait a bit so the callback can happen before we stop
  co_await ctx.timers().sleep_for(std::chrono::milliseconds(250));

  std::cout << "[cnerium] timer demo done\n";
  ctx.stop();
  co_return;
}

int main()
{
  io_context ctx;

  auto t = app(ctx);
  ctx.post(t.handle());

  ctx.run();
  return 0;
}
