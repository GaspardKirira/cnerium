/**
 *
 *  @file udp.hpp
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
#ifndef VIX_ASYNC_UDP_HPP
#define VIX_ASYNC_UDP_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <system_error>

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
   * @brief UDP endpoint description.
   *
   * Represents a UDP network endpoint defined by a hostname (or IP string)
   * and a UDP port.
   */
  struct udp_endpoint
  {
    /**
     * @brief Hostname or IP address.
     *
     * Examples: "localhost", "127.0.0.1", "::1".
     */
    std::string host;

    /**
     * @brief UDP port number in host byte order.
     */
    std::uint16_t port{0};
  };

  /**
   * @brief Result of a UDP receive operation.
   *
   * Contains metadata about the received datagram, including the sender
   * endpoint and the number of bytes written into the receive buffer.
   */
  struct udp_datagram
  {
    /**
     * @brief Sender endpoint.
     */
    udp_endpoint from;

    /**
     * @brief Number of bytes received.
     */
    std::size_t bytes{0};
  };

  /**
   * @brief Abstract asynchronous UDP socket interface.
   *
   * udp_socket defines the coroutine-friendly contract for UDP networking.
   * Implementations are runtime-specific (typically Asio-backed) and integrate
   * with vix::async::core::io_context.
   *
   * Supported operations:
   * - bind to a local endpoint
   * - send datagrams to a remote endpoint
   * - receive datagrams from any remote endpoint
   */
  class udp_socket
  {
  public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~udp_socket() = default;

    /**
     * @brief Asynchronously bind the socket to a local UDP endpoint.
     *
     * @param bind_ep Local endpoint to bind to.
     *
     * @return task<void> that completes once the socket is bound.
     *
     * @throws std::system_error on bind failure.
     */
    virtual core::task<void> async_bind(
        const udp_endpoint &bind_ep) = 0;

    /**
     * @brief Asynchronously send a datagram to a remote endpoint.
     *
     * @param buf Data buffer to send.
     * @param to Destination endpoint.
     * @param ct Optional cancellation token.
     *
     * @return task<std::size_t> Number of bytes actually sent.
     *
     * @throws std::system_error on send failure or cancellation.
     */
    virtual core::task<std::size_t> async_send_to(
        std::span<const std::byte> buf,
        const udp_endpoint &to,
        core::cancel_token ct = {}) = 0;

    /**
     * @brief Asynchronously receive a datagram from the socket.
     *
     * Writes incoming data into the provided buffer and returns metadata
     * describing the sender and received size.
     *
     * @param buf Destination buffer for received data.
     * @param ct Optional cancellation token.
     *
     * @return task<udp_datagram> Metadata of the received datagram.
     *
     * @throws std::system_error on receive failure or cancellation.
     */
    virtual core::task<udp_datagram> async_recv_from(
        std::span<std::byte> buf,
        core::cancel_token ct = {}) = 0;

    /**
     * @brief Close the UDP socket.
     *
     * This operation is idempotent and may be called multiple times.
     */
    virtual void close() noexcept = 0;

    /**
     * @brief Check whether the socket is currently open.
     *
     * @return true if the socket is open, false otherwise.
     */
    virtual bool is_open() const noexcept = 0;
  };

  /**
   * @brief Create a UDP socket associated with an io_context.
   *
   * @param ctx Core io_context used for scheduling and integration.
   * @return Unique pointer owning a udp_socket instance.
   */
  std::unique_ptr<udp_socket> make_udp_socket(core::io_context &ctx);

} // namespace vix::async::net

#endif // VIX_ASYNC_UDP_HPP
