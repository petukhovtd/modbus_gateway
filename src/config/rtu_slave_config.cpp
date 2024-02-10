#include <config/extractor.h>
#include <config/keys.h>
#include <config/rtu_slave_config.h>

namespace modbus_gateway {
RtuSlaveConfig::RtuSlaveConfig(TracePath &tracePath, const nlohmann::json::value_type &obj, modbus::FrameType frameType)
    : ITransportConfig(TransportType::RtuSlave),
      frameType_(frameType) {
  device = ExtractString(tracePath, obj, keys::device);
  rtuOptions = ExtractRtuOptions(tracePath, obj);
}
modbus::FrameType RtuSlaveConfig::GetFrameType() const {
  return frameType_;
}
}// namespace modbus_gateway