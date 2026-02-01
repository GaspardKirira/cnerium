#include <cassert>
#include <iostream>
#include <stdexcept>

#include <cnerium/core/task.hpp>

using cnerium::core::task;

static task<int> compute_value()
{
  co_return 42;
}

static task<int> add_one(int x)
{
  co_return x + 1;
}

static task<int> chain()
{
  int v = co_await compute_value();
  int r = co_await add_one(v);
  co_return r; // 43
}

static task<void> throws_task()
{
  throw std::runtime_error("boom");
  co_return;
}

static task<void> chain_void()
{
  int r = co_await chain();
  assert(r == 43);

  co_return;
}

// Helper: run a task to completion without a scheduler.
// This works for our current task implementation because awaiting a task transfers execution
// to the task coroutine and continues via continuations until completion.
template <typename T>
static T sync_await(task<T> t)
{
  // Wrap in a small coroutine so we can co_await and capture the result.
  struct runner
  {
    task<T> inner;
    T value{};

    task<void> run()
    {
      if constexpr (std::is_void_v<T>)
      {
        co_await inner;
        co_return;
      }
      else
      {
        value = co_await inner;
        co_return;
      }
    }
  };

  runner r{std::move(t)};

  // Start the runner coroutine and resume until done.
  auto h = r.run().handle();
  assert(h);

  while (!h.done())
    h.resume();

  if constexpr (std::is_void_v<T>)
  {
    return;
  }
  else
  {
    return std::move(r.value);
  }
}

int main()
{
  // Basic chain
  {
    int v = sync_await(chain());
    assert(v == 43);
  }

  // Void chain
  {
    sync_await(chain_void());
  }

  // Exception propagation
  {
    bool caught = false;
    try
    {
      sync_await(throws_task());
    }
    catch (const std::runtime_error &e)
    {
      caught = true;
      // Keep message check loose, but ensure it is our exception.
      assert(std::string(e.what()).find("boom") != std::string::npos);
    }
    assert(caught);
  }

  std::cout << "cnerium_task_smoke: OK\n";
  return 0;
}
