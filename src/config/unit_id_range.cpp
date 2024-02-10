#include <config/unit_id_range.h>
#include <config/config_types.h>
#include <config/extractor.h>
#include <config/keys.h>


namespace modbus_gateway {

UnitIdRange::UnitIdRange(TracePath &tracePath, const nlohmann::json::value_type &obj) {
  const NumericRangeType type = ExtractNumericRangeType(tracePath, obj);
  switch (type) {
  case NumericRangeType::Range:
    begin = ExtractModbusUnitId(tracePath, obj, keys::numericRangeBegin);
    end = ExtractModbusUnitId(tracePath, obj, keys::numericRangeEnd);
    Normalize();
    break;
  case NumericRangeType::Value:
    begin = end = ExtractModbusUnitId(tracePath, obj, keys::numericRangeValue);
    break;
  default:
    throw std::logic_error("unsupported range type");
  }
}

void UnitIdRange::Normalize() {
  if (begin > end) {
    std::swap(begin, end);
  }
}

}// namespace modbus_gateway
