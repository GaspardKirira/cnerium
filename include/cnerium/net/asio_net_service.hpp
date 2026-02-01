#pragma once

#include <memory>
#include <thread>

#include <asio/io_context.hpp>
#include <asio/executor_work_guard.hpp>

namespace cnerium::core
{
  class io_context;
}

namespace cnerium::net::detail
{

  class asio_net_service
  {
  public:
    explicit asio_net_service(cnerium::core::io_context &ctx);
    ~asio_net_service();

    asio_net_service(const asio_net_service &) = delete;
    asio_net_service &operator=(const asio_net_service &) = delete;

    asio::io_context &asio_ctx() noexcept { return ioc_; }

    void stop() noexcept;

  private:
    cnerium::core::io_context &ctx_;

    asio::io_context ioc_;
    using guard_t = asio::executor_work_guard<asio::io_context::executor_type>;
    std::unique_ptr<guard_t> guard_;

    std::thread net_thread_;
    bool stopped_{false};
  };

} // namespace cnerium::net::detail
