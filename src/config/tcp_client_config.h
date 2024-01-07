#pragma once

#include <config/i_transport_config.h>
#include <config/trace_path.h>
#include <config/unit_id_range.h>

#include <common/types_asio.h>

#include <nlohmann/json.hpp>

namespace modbus_gateway {

struct TcpClientConfig : public ITransportConfig {
public:
  TcpClientConfig(TracePath &tracePath, const nlohmann::json::value_type &obj);

  ~TcpClientConfig() override = default;

  asio::ip::address address = asio::ip::address_v4::any();
  asio::ip::port_type port = 502;
  std::chrono::milliseconds timeout = std::chrono::milliseconds(1000);
  std::vector<UnitIdRange> unitIdSet{};
};

}// namespace modbus_gateway
