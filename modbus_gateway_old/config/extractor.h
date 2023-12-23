#ifndef MODBUS_GATEWAY_EXTRACTOR_H
#define MODBUS_GATEWAY_EXTRACTOR_H

#include <config/value_type.h>
#include <config/i_transport_config.h>
#include <config/invalid_value_exception.h>
#include <config/config_types.h>
#include <common/fmt_logger.h>
#include <types.h>

#include <nlohmann/json.hpp>

namespace modbus_gateway
{

class TraceDeep;
class TracePath;
class UnitIdRange;

const nlohmann::json::value_type& FindObject( const TraceDeep& td, const nlohmann::json::value_type& obj );

void CheckType( const TraceDeep& td, const nlohmann::json::value_type& obj, ValueType expectType );

std::optional< FmtLogger::LogLevel > ExtractLogLevel( TracePath& tracePath, const nlohmann::json::value_type& obj );

std::optional< asio::ip::address > ExtractIpAddressOptional( TracePath& tracePath, const nlohmann::json::value_type& obj );

asio::ip::address ExtractIpAddress( TracePath& tracePath, const nlohmann::json::value_type& obj );

asio::ip::port_type ExtractIpPort( TracePath& tracePath, const nlohmann::json::value_type& obj );

TransportType ExtractTransportType( TracePath& tracePath, const nlohmann::json::value_type& obj );

TransportConfigPtr ExtractServer( TracePath& tracePath, const nlohmann::json::value_type& obj );

std::vector< TransportConfigPtr > ExtractServers( TracePath& tracePath, const nlohmann::json::value_type& obj );

template< typename T >
T ExtractUnsignedNumber( TracePath& tracePath, const nlohmann::json::value_type& obj, const std::string& key )
{
     TraceDeep td( tracePath, key );

     const auto& val = FindObject( td, obj );
     CheckType( td, val, ValueType::Number );

     const auto value = val.get< uint64_t >();
     if( value > std::numeric_limits< T >::max() )
     {
          throw InvalidValueException( td, std::to_string( value ) );
     }

     return static_cast< T >( value );
}

modbus::UnitId ExtractModbusUnitId( TracePath& tracePath, const nlohmann::json::value_type& obj, const std::string& key );

NumericRangeType ExtractNumericRangeType( TracePath& tracePath, const nlohmann::json::value_type& obj );

std::optional< std::vector< UnitIdRange > > ExtractUnitIdRangeSet( TracePath& tracePath, const nlohmann::json::value_type& obj );

TransportConfigPtr ExtractClient( TracePath& tracePath, const nlohmann::json::value_type& obj );

std::vector< TransportConfigPtr > ExtractClients( TracePath& tracePath, const nlohmann::json::value_type& obj );

}


#endif
