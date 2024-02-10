#include <config/extractor.h>
#include <config/keys.h>
#include <config/tcp_server_config.h>

namespace modbus_gateway {

TcpServerConfig::TcpServerConfig(TracePath &tracePath, const nlohmann::json::value_type::value_type &obj)
    : ITransportConfig(TransportType::TcpServer) {

  const auto addrOpt = ExtractIpAddressOptional(tracePath, obj);
  if (addrOpt.has_value()) {
    address = addrOpt.value();
  }

  port = ExtractIpPort(tracePath, obj);
}

}// namespace modbus_gateway
