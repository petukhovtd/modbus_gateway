#ifndef MODBUS_GATEWAY_TYPES_H
#define MODBUS_GATEWAY_TYPES_H

#include <asio.hpp>
#include <vector>

namespace modbus_gateway
{

using ContextPtr = std::shared_ptr< asio::io_context >;
using SocketPtr = std::shared_ptr< asio::ip::tcp::socket >;

using Data = std::vector< uint8_t >;
using DataPtr = std::shared_ptr< Data >;
static constexpr const size_t DataSize = 0xFFFF;

}

#endif //MODBUS_GATEWAY_TYPES_H
