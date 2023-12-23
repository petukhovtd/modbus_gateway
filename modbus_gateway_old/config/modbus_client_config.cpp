#include "modbus_client_config.h"
#include "extractor.h"

namespace modbus_gateway
{
ModbusClientConfig::ModbusClientConfig( TracePath& tracePath, const nlohmann::json::value_type& obj )
: unitIds_()
{
     const auto idsOpt = ExtractUnitIdRangeSet( tracePath, obj );
     if( idsOpt.has_value() )
     {
          unitIds_ = idsOpt.value();
          CompressUnitIds();
     }
}

ModbusClientConfig::ModbusClientConfig( const std::vector< UnitIdRange >& unitIds )
: unitIds_( unitIds )
{
     CompressUnitIds();
}

void ModbusClientConfig::CompressUnitIds()
{
     CompressUnitIdRanges( unitIds_ );
}

const std::vector< UnitIdRange >& ModbusClientConfig::GetUnitIds() const
{
     return unitIds_;
}

}