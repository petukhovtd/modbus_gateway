#ifndef MODBUS_GATEWAY_TYPES_H
#define MODBUS_GATEWAY_TYPES_H

#include <asio.hpp>
#include <asio/basic_waitable_timer.hpp>
#include <modbus/modbus_buffer.h>
#include <i_router.h>

namespace modbus_gateway
{

using ContextPtr = std::shared_ptr< asio::io_context >;
using WorkPtr = std::shared_ptr< asio::executor_work_guard< asio::io_context > >;
using TcpSocketPtr = std::shared_ptr< asio::ip::tcp::socket >;
using TcpSocketUPtr = std::unique_ptr< asio::ip::tcp::socket >;
using TcpEndpoint = asio::ip::tcp::endpoint;
using TcpAcceptor = asio::ip::tcp::acceptor;
using TcpAcceptorPtr = std::unique_ptr< TcpAcceptor >;
using WaitTimerPtr = std::unique_ptr< asio::basic_waitable_timer< std::chrono::steady_clock > >;

using ModbusBufferPtr = std::shared_ptr< modbus::ModbusBuffer >;

using RouterPtr = std::shared_ptr< IRouter >;

using TimeoutMs = std::chrono::milliseconds;

enum TransportType
{
     TCP,
};

}

#endif //MODBUS_GATEWAY_TYPES_H