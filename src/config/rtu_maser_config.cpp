#include <config/extractor.h>
#include <config/keys.h>
#include <config/rtu_maser_config.h>

namespace modbus_gateway {

RtuMasterConfig::RtuMasterConfig(TracePath &tracePath, const nlohmann::json::value_type &obj, modbus::FrameType frameType)
    : ITransportConfig(TransportType::RtuMaster),
      frameType_(frameType) {

  device = ExtractString(tracePath, obj, keys::device);
  rtuOptions = ExtractRtuOptions(tracePath, obj);
  timeout = std::chrono::milliseconds(ExtractUnsignedNumber<size_t>(tracePath, obj, keys::timeout));

  const auto unitIdSetOpt = ExtractUnitIdRangeSet(tracePath, obj);
  if (unitIdSetOpt.has_value()) {
    unitIdSet = unitIdSetOpt.value();
  }
}

modbus::FrameType RtuMasterConfig::GetFrameType() const {
  return frameType_;
}

}// namespace modbus_gateway