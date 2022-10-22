#ifndef MODBUS_GATEWAY_MODBUS_CLIENT_CONFIG_H
#define MODBUS_GATEWAY_MODBUS_CLIENT_CONFIG_H

#include <config/trace_path.h>
#include <config/unit_id_range.h>

#include <nlohmann/json.hpp>

namespace modbus_gateway
{

class ModbusClientConfig
{
public:
     ModbusClientConfig( TracePath& tracePath, const nlohmann::json::value_type& obj );

     explicit ModbusClientConfig( const std::vector< UnitIdRange >& unitIds );

     virtual ~ModbusClientConfig() = default;

     void CompressUnitIds();

     const std::vector< UnitIdRange >& GetUnitIds() const;

private:
     std::vector< UnitIdRange > unitIds_;
};

using ClientConfigPtr = std::shared_ptr< ModbusClientConfig >;

}

#endif //MODBUS_GATEWAY_MODBUS_CLIENT_CONFIG_H
