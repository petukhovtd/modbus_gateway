#include "extractor.h"
#include "invalid_type_exception.h"
#include "invalid_value_exception.h"
#include "key_not_found_exception.h"
#include "trace_deep.h"
#include "convertors.h"
#include "unit_id_range.h"
#include "tcp_server_config.h"
#include "tcp_client_config.h"
#include "keys.h"

namespace modbus_gateway
{

const nlohmann::json::value_type& FindObject( const TraceDeep& td, const nlohmann::json::value_type& obj )
{
     const auto& it = obj.find( td.GetKey() );
     if( obj.end() == it )
     {
          throw KeyNotFoundException( td );
     }
     return *it;
}

void CheckType( const TraceDeep& td, const nlohmann::json::value_type& obj, ValueType expectType )
{
     const ValueType actualType = ConvertJsonTypeToValueType( obj.type() );
     if( actualType != expectType )
     {
          throw InvalidTypeException( td, expectType, actualType );
     }
}

std::optional< FmtLogger::LogLevel > ExtractLogLevel( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
     TraceDeep td( tracePath, keys::logLevel );

     const auto it = obj.find( keys::logLevel );
     if( obj.end() == it )
     {
          return std::nullopt;
     }
     const auto& val = it.value();
     CheckType( td, val, ValueType::String );

     const auto& value = val.get< std::string >();
     const auto convertValue = ConvertLogLevel( value );
     if( !convertValue.has_value() )
     {
          throw InvalidValueException( td, value );
     }
     return convertValue.value();
}

std::optional< asio::ip::address > ExtractIpAddressOptional( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
     TraceDeep td( tracePath, keys::ipAddress );

     const auto& it = obj.find( keys::ipAddress );
     if( obj.end() == it )
     {
          return std::nullopt;
     }
     const auto& val = it.value();

     CheckType( td, val, ValueType::String );

     const auto& value = val.get< std::string >();
     asio::error_code ec;
     const auto address = asio::ip::make_address_v4( value, ec );
     if( ec )
     {
          throw InvalidValueException( td, value );
     }
     return address;
}

asio::ip::address ExtractIpAddress( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
     const auto& res = ExtractIpAddressOptional( tracePath, obj );
     if( !res.has_value() )
     {
          throw KeyNotFoundException( TraceDeep( tracePath, keys::ipAddress ) );
     }

     return res.value();
}

asio::ip::port_type ExtractIpPort( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
     TraceDeep td( tracePath, keys::ipPort );

     const auto& val = FindObject( td, obj );
     CheckType( td, val, ValueType::Number );

     const auto value = val.get< uint64_t >();
     if( value == 0 || value > std::numeric_limits< asio::ip::port_type >::max() )
     {
          throw InvalidValueException( td, std::to_string( value ) );
     }

     return static_cast< asio::ip::port_type >( value );
}

TransportType ExtractTransportType( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
     TraceDeep td( tracePath, keys::transportType );

     const auto& val = FindObject( td, obj );
     CheckType( td, val, ValueType::String );

     const auto& value = val.get< std::string >();
     const auto convertValue = ConvertTransportType( value );
     if( !convertValue.has_value() )
     {
          throw InvalidValueException( td, value );
     }
     return convertValue.value();
}

TransportConfigPtr ExtractServer( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
     TransportType transportType = ExtractTransportType( tracePath, obj );
     switch( transportType )
     {
          case TCP: return std::make_shared< TcpServerConfig >( tracePath, obj );
          default:
               throw std::logic_error( "unimplemented transport type" );
     }
}

std::vector< TransportConfigPtr > ExtractServers( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
     TraceDeep td( tracePath, keys::servers );

     const auto& servers = FindObject( td, obj );
     CheckType( td, servers, ValueType::Array );

     if( servers.empty() )
     {
          throw InvalidValueException( td, keys::servers + " empty" );
     }

     size_t index = 0;
     std::vector< TransportConfigPtr > result;
     result.reserve( servers.size() );
     for( const auto& server: servers )
     {
          TraceDeep tdServer( td.GetTracePath(), std::to_string( index ) );
          result.push_back( ExtractServer( tdServer.GetTracePath(), server ) );
          ++index;
     }

     return result;
}

modbus::UnitId ExtractModbusUnitId( TracePath& tracePath, const nlohmann::json::value_type& obj, const std::string& key )
{
     const auto unitId = ExtractUnsignedNumber< modbus::UnitId >( tracePath, obj, key );
     if( unitId < modbus::unitIdMin || unitId > modbus::unitIdMax )
     {
          TraceDeep td( tracePath, key );
          throw InvalidValueException( td, std::to_string( unitId ) );
     }
     return unitId;
}

NumericRangeType ExtractNumericRangeType( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
     TraceDeep td( tracePath, keys::numericRangeType );

     const auto& val = FindObject( td, obj );
     CheckType( td, val, ValueType::String );

     const auto value = val.get< std::string >();
     const auto convertValue = ConvertNumericRangeType( value );
     if( !convertValue.has_value() )
     {
          throw InvalidValueException( td, value );
     }
     return convertValue.value();
}

std::optional< std::vector< UnitIdRange > > ExtractUnitIdRangeSet( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
     TraceDeep td( tracePath, keys::unitIdSet );

     const auto& it = obj.find( keys::unitIdSet );
     if( obj.end() == it )
     {
          return std::nullopt;
     }
     const auto& unitIds = *it;
     CheckType( td, unitIds, ValueType::Array );

     if( unitIds.empty() )
     {
          throw InvalidValueException( td, keys::unitIdSet + " empty" );
     }

     size_t index = 0;
     std::vector< UnitIdRange > result;
     result.reserve( unitIds.size() );
     for( const auto& unitIdRange: unitIds )
     {
          TraceDeep tdUnitIdRange( td.GetTracePath(), std::to_string( index ) );
          result.emplace_back( tdUnitIdRange.GetTracePath(), unitIdRange );
          ++index;
     }

     return result;
}

TransportConfigPtr ExtractClient( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
     TransportType transportType = ExtractTransportType( tracePath, obj );
     switch( transportType )
     {
          case TCP: return std::make_shared< TcpClientConfig >( tracePath, obj );
          default:
               throw std::logic_error( "unimplemented transport type" );
     }
}

std::vector< TransportConfigPtr > ExtractClients( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
     TraceDeep td( tracePath, keys::clients );

     const auto& servers = FindObject( td, obj );
     CheckType( td, servers, ValueType::Array );

     if( servers.empty() )
     {
          throw InvalidValueException( td, keys::clients + " empty" );
     }

     size_t index = 0;
     std::vector< TransportConfigPtr > result;
     result.reserve( servers.size() );
     for( const auto& server: servers )
     {
          TraceDeep tdServer( td.GetTracePath(), std::to_string( index ) );
          result.push_back( ExtractClient( tdServer.GetTracePath(), server ) );
          ++index;
     }

     return result;
}

}
