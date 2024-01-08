#include <config/tcp_client_config.h>

#include <config/extractor.h>

namespace modbus_gateway {

TcpClientConfig::TcpClientConfig(TracePath &tracePath, const nlohmann::json::value_type &obj)
    : ITransportConfig(TransportType::TcpClient),
      MasterConfig(tracePath, obj) {
  address = ExtractIpAddress(tracePath, obj);
  port = ExtractIpPort(tracePath, obj);
}

}// namespace modbus_gateway
