#include <config/extractor.h>
#include <config/keys.h>
#include <config/tcp_client_config.h>

namespace modbus_gateway {
TcpClientConfig::TcpClientConfig(TracePath &tracePath, const nlohmann::json::value_type &obj)
    : ITransportConfig(TransportType::TcpClient) {
  address = ExtractIpAddress(tracePath, obj);
  port = ExtractIpPort(tracePath, obj);
  timeout = std::chrono::milliseconds(ExtractUnsignedNumber<size_t>(tracePath, obj, keys::timeout));

  const auto unitIdSetOpt = ExtractUnitIdRangeSet(tracePath, obj);
  if (unitIdSetOpt.has_value()) {
    unitIdSet = unitIdSetOpt.value();
  }
}

}// namespace modbus_gateway
