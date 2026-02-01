#include <iostream>
#include <vector>

#include <cnerium/core/io_context.hpp>
#include <cnerium/core/task.hpp>
#include <cnerium/core/signal.hpp>
#include <cnerium/core/spawn.hpp>

#include <cnerium/net/tcp.hpp>

using cnerium::core::io_context;
using cnerium::core::task;

static task<void> handle_client(std::unique_ptr<cnerium::net::tcp_stream> client)
{
  std::cout << "[cnerium] client connected\n";

  std::vector<std::byte> buf(4096);

  while (client && client->is_open())
  {
    std::size_t n = 0;

    try
    {
      n = co_await client->async_read(
          std::span<std::byte>(buf.data(), buf.size()));
    }
    catch (const std::system_error &e)
    {
      std::cout << "[cnerium] read error: " << e.code().message() << "\n";
      break;
    }

    if (n == 0)
      break;

    try
    {
      co_await client->async_write(
          std::span<const std::byte>(buf.data(), n));
    }
    catch (const std::system_error &e)
    {
      std::cout << "[cnerium] write error: " << e.code().message() << "\n";
      break;
    }
  }

  client->close();
  std::cout << "[cnerium] client disconnected\n";
  co_return;
}

static task<void> server(io_context &ctx)
{
  auto &sig = ctx.signals();
  sig.add(SIGINT);
  sig.add(SIGTERM);
  sig.on_signal([&](int s)
                {
    std::cout << "[cnerium] signal " << s << " received -> stopping\n";
    ctx.stop(); });

  auto listener = cnerium::net::make_tcp_listener(ctx);

  co_await listener->async_listen({"0.0.0.0", 9090}, 128);
  std::cout << "[cnerium] echo server listening on 0.0.0.0:9090\n";

  while (ctx.is_running())
  {
    try
    {
      auto client = co_await listener->async_accept();

      // 🔥 SPAWN CONCURRENT CLIENT HANDLER
      cnerium::core::spawn_detached(
          ctx,
          handle_client(std::move(client)));
    }
    catch (const std::system_error &e)
    {
      std::cout << "[cnerium] accept error: " << e.code().message() << "\n";
      break;
    }
  }

  listener->close();
  ctx.stop();
  co_return;
}

int main()
{
  io_context ctx;

  auto t = server(ctx);
  ctx.post(t.handle());

  ctx.run();
  std::cout << "[cnerium] server stopped\n";
  return 0;
}
