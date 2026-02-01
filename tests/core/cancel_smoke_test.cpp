#include <cassert>
#include <iostream>

#include <cnerium/core/cancel.hpp>

using cnerium::core::cancel_source;
using cnerium::core::cancel_token;

static void test_default_token()
{
  cancel_token ct;
  assert(!ct.can_cancel());
  assert(!ct.is_cancelled());
}

static void test_cancel_flow()
{
  cancel_source src;
  auto ct = src.token();

  assert(ct.can_cancel());
  assert(!ct.is_cancelled());
  assert(!src.is_cancelled());

  src.request_cancel();

  assert(ct.is_cancelled());
  assert(src.is_cancelled());
}

int main()
{
  test_default_token();
  test_cancel_flow();

  std::cout << "cnerium_cancel_smoke: OK\n";
  return 0;
}
