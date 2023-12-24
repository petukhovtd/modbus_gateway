#pragma once

#include <asio.hpp>

namespace modbus_gateway {

using TcpSocketPtr = std::unique_ptr<asio::ip::tcp::socket>;
using ContextPtr = std::shared_ptr<asio::io_context>;
using TcpAcceptor = asio::ip::tcp::acceptor;
using TcpEndpoint = asio::ip::tcp::endpoint;

}