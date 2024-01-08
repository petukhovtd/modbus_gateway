#pragma once

#include <common/types_asio.h>
#include <config/i_transport_config.h>
#include <config/trace_path.h>

#include <nlohmann/json.hpp>

namespace modbus_gateway {

struct TcpServerConfig : public ITransportConfig {
public:
  explicit TcpServerConfig(TracePath &tracePath, const nlohmann::json::value_type &obj);

  ~TcpServerConfig() override = default;

  asio::ip::address address = asio::ip::address_v4::any();
  asio::ip::port_type port = 502;
};

}// namespace modbus_gateway
