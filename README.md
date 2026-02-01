# Cnerium

**Cnerium** is a **modern asynchronous runtime for C++20**, inspired by Node.js, libuv, and Asio, but **designed natively for C++** with:

- **C++20 coroutines (`co_await`)**
- an explicit **event loop**
- a deterministic **scheduler**
- a **CPU thread pool**
- **non-blocking timers**
- an **Asio-based networking backend**
- a **stable async API** for building fully asynchronous C++ libraries

> The goal of Cnerium is to provide a **minimal, explicit, and extensible async core** on top of which real C++ runtimes, servers, and libraries can be built.

---

## ✨ Core Principles

- **Async-first**: no blocking APIs
- **Single event loop** (Node/libuv-style)
- **Clear separation of concerns**:
  - Event loop (scheduler)
  - CPU work (thread pool)
  - I/O (network backend)
- **C++20 coroutines** as the primary abstraction
- **Explicit cancellation** (`cancel_token`)
- **No hidden magic**

---

## 🚀 Minimal Example

```cpp
#include <cnerium/core/io_context.hpp>
#include <cnerium/core/task.hpp>
#include <cnerium/core/timer.hpp>

using cnerium::core::io_context;
using cnerium::core::task;

task<void> app(io_context& ctx)
{
  co_await ctx.timers().sleep_for(std::chrono::milliseconds(100));
  ctx.stop();
}

int main()
{
  io_context ctx;
  auto t = app(ctx);
  ctx.post(t.handle());
  ctx.run();
}
```
🧠 Execution Model

1 event loop thread
N CPU worker threads
1 network I/O thread (Asio)
All coroutine resumptions return to the event loop
```css
[ event loop ]
     ↑   ↓
[ coroutines ]
     ↑   ↓
[ timers / net / cpu pool ]
```
This model guarantees:
- no race conditions on user code
- deterministic execution
- predictable performance

🌐 Networking (TCP, UDP, DNS)
Cnerium exposes backend-agnostic async networking interfaces.

## TCP example
```cpp
auto listener = cnerium::net::make_tcp_listener(ctx);
co_await listener->async_listen({"0.0.0.0", 9090});

auto client = co_await listener->async_accept();
std::size_t n = co_await client->async_read(buffer);
co_await client->async_write(buffer.subspan(0, n));
```
Current backend:
- Asio standalone, running on a dedicated network thread

## 🔁 Concurrency: spawn_detached
To run an independent coroutine (TCP client, background job, etc.):
```cpp
#include <cnerium/core/spawn.hpp>
cnerium::core::spawn_detached(ctx, handle_client(std::move(client)));
```
### Properties:

- fully detached
- no memory leaks
- no use-after-free
- automatic destruction at coroutine completion

## 🛑 Cancellation

```cpp
cancel_source src;
auto ct = src.token();

co_await ctx.timers().sleep_for(1s, ct);

src.request_cancel(); // cooperative cancellation
```
Cancellation is:
- explicit
- composable
- non-intrusive

## 🧪 Tests

Included smoke tests:
```sql
tests/
├── core/
│   ├── task_smoke_test.cpp
│   ├── cancel_smoke_test.cpp
│   └── scheduler_smoke_test.cpp
```
### Run tests:
```bash
cmake -S . -B build -DCNERIUM_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```
## 📚 Examples
```sql
examples/
├── 00_hello_task.cpp
├── 01_timer.cpp
├── 02_thread_pool.cpp
└── 03_tcp_echo_server.cpp
```
## Build examples:
```bash
cmake -S . -B build -DCNERIUM_BUILD_EXAMPLES=ON
cmake --build build
```
## 🛠️ Build Requirements

- C++20 compiler
- CMake ≥ 3.20
- Asio (standalone, via FetchContent)

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

🧭 Roadmap

when_all / task aggregation
network backpressure
hierarchical cancellation
structured logging
metrics and tracing
alternative networking backends
Windows IOCP support

📜 License MIT

💡 Vision

Cnerium is not a framework.
It is a low-level async runtime foundation for building:

- servers
- runtimes
- networking stacks
- fully asynchronous C++ libraries
- Everything is explicit, deterministic, and under developer control.
