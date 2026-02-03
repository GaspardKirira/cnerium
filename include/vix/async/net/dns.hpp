/**
 *
 *  @file dns.hpp
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
#ifndef VIX_ASYNC_DNS_HPP
#define VIX_ASYNC_DNS_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <vix/async/core/task.hpp>
#include <vix/async/core/cancel.hpp>
#include <vix/async/core/error.hpp>

namespace vix::async::core
{
  class io_context;
}

namespace vix::async::net
{
  /**
   * @brief Result of a DNS resolution.
   *
   * Represents a single resolved network endpoint returned by a DNS backend.
   * The address is stored as a textual IP representation (IPv4 or IPv6),
   * along with the resolved port number.
   */
  struct resolved_address
  {
    /**
     * @brief IP address as a string.
     *
     * Examples: "127.0.0.1", "192.168.1.10", "::1".
     */
    std::string ip;

    /**
     * @brief Network port in host byte order.
     */
    std::uint16_t port{0};
  };

  /**
   * @brief Abstract asynchronous DNS resolver interface.
   *
   * dns_resolver defines the contract for hostname resolution within the
   * vix async runtime. Implementations may rely on different backends
   * (e.g. Asio, system resolver, custom cache, etc.) but must expose a
   * coroutine-friendly API returning task<>.
   *
   * The resolver:
   * - resolves a hostname and port into one or more IP endpoints
   * - supports cooperative cancellation via cancel_token
   * - reports errors via exceptions or error codes wrapped in task<>
   */
  class dns_resolver
  {
  public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~dns_resolver() = default;

    /**
     * @brief Resolve a hostname and port asynchronously.
     *
     * This function performs DNS resolution and returns all resolved
     * addresses as a vector of resolved_address entries.
     *
     * @param host Hostname to resolve (e.g. "example.com").
     * @param port Port number associated with the service.
     * @param ct Optional cancellation token.
     *
     * @return task<std::vector<resolved_address>> containing resolved endpoints.
     *
     * @throws std::system_error or runtime-specific errors on failure.
     */
    virtual core::task<std::vector<resolved_address>> async_resolve(
        std::string host,
        std::uint16_t port,
        core::cancel_token ct = {}) = 0;
  };

  /**
   * @brief Create the default DNS resolver for the current runtime.
   *
   * This factory returns a concrete dns_resolver implementation suitable
   * for the given io_context. The actual backend is runtime-defined
   * (typically Asio-based).
   *
   * @param ctx Core io_context used for scheduling and integration.
   * @return Unique pointer owning a dns_resolver instance.
   */
  std::unique_ptr<dns_resolver> make_dns_resolver(core::io_context &ctx);

} // namespace vix::async::net

#endif // VIX_ASYNC_DNS_HPP
