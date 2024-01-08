#include <config/master_config.h>
#include <config/extractor.h>
#include <config/keys.h>

namespace modbus_gateway
{
MasterConfig::MasterConfig( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
  timeout = std::chrono::milliseconds(ExtractUnsignedNumber<size_t>(tracePath, obj, keys::timeout));

  const auto unitIdSetOpt = ExtractUnitIdRangeSet(tracePath, obj);
  if (unitIdSetOpt.has_value()) {
    unitIdSet = unitIdSetOpt.value();
  }
}

}