#ifndef MODBUS_GATEWAY_UNIT_ID_RANGE_H
#define MODBUS_GATEWAY_UNIT_ID_RANGE_H

#include <config/trace_deep.h>
#include <modbus/modbus_types.h>

#include <nlohmann/json.hpp>

namespace modbus_gateway
{

struct UnitIdRange
{
     modbus::UnitId begin;
     modbus::UnitId end;

     UnitIdRange( TracePath& tracePath, const nlohmann::json::value_type& obj );

     UnitIdRange( modbus::UnitId begin, modbus::UnitId end );

     explicit  UnitIdRange( modbus::UnitId value );

     void Normalize();
};

void CompressUnitIdRanges( std::vector< UnitIdRange >& unitIds );

}

#endif //MODBUS_GATEWAY_UNIT_ID_RANGE_H
