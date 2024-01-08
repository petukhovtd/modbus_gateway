#include <config/rtu_maser_config.h>

#include <config/extractor.h>
#include <config/keys.h>

namespace modbus_gateway {

RtuMasterConfig::RtuMasterConfig(TracePath &tracePath, const nlohmann::json::value_type &obj, modbus::FrameType frameType)
    : ITransportConfig(TransportType::RtuMaster),
      MasterConfig(tracePath,obj),
      frameType_(frameType) {

  device = ExtractString(tracePath, obj, keys::device);
  rtuOptions = ExtractRtuOptions(tracePath, obj);
}

modbus::FrameType RtuMasterConfig::GetFrameType() const {
  return frameType_;
}

}// namespace modbus_gateway