#include "validators.h"
#include "trace_path.h"
#include "trace_deep.h"
#include "tcp_server_config.h"
#include "tcp_client_config.h"
#include "invalid_value_exception.h"

#include <map>
#include <set>

namespace modbus_gateway
{

void ValidateServers( TracePath& tracePath, const std::vector< TransportConfigPtr >& servers )
{
     using AddressSet = std::vector< asio::ip::address >;
     std::map< asio::ip::port_type, AddressSet > tcpServerEnpoints;

     TraceDeep td( tracePath, "servers" );

     size_t index = 0;
     for( const auto& server: servers )
     {
          TraceDeep tdServer( tracePath, std::to_string( index ) );
          switch( server->GetType() )
          {
               case TCP:
               {
                    const auto& serverPtr = std::dynamic_pointer_cast< TcpServerConfig >( server );
                    if( !serverPtr )
                    {
                         throw std::runtime_error( "cast to TcpServerConfig failed" );
                    }
                    auto it = tcpServerEnpoints.find( serverPtr->GetPort() );
                    if( it == tcpServerEnpoints.end() )
                    {
                         if( serverPtr->GetAddress() == asio::ip::address_v4::any() )
                         {
                              // any address
                              tcpServerEnpoints[ serverPtr->GetPort() ] = AddressSet();
                         }
                         else
                         {
                              // ip address
                              tcpServerEnpoints[ serverPtr->GetPort() ] = AddressSet { serverPtr->GetAddress() };
                         }
                    }
                    else
                    {
                         AddressSet& addressSet = it->second;

                         if( serverPtr->GetAddress() == asio::ip::address_v4::any() )
                         {
                              std::stringstream ss;
                              ss << "any ip address intersected with ip address on other server";
                              throw InvalidValueException( tdServer, ss.str() );
                         }

                         if( addressSet.empty() )
                         {
                              std::stringstream ss;
                              ss << "ip address " << serverPtr->GetAddress().to_string()
                                 << " intersected with any address on other server";
                              throw InvalidValueException( tdServer, ss.str() );
                         }

                         const auto addrIt = std::find( addressSet.begin(), addressSet.end(), serverPtr->GetAddress() );
                         if( addrIt != addressSet.end() )
                         {
                              std::stringstream ss;
                              ss << "ip address " << serverPtr->GetAddress().to_string()
                                 << " intersected with ip address " << addrIt->to_string() << " on other server";
                              throw InvalidValueException( tdServer, ss.str() );
                         }

                         addressSet.push_back( serverPtr->GetAddress() );
                    }
               }
                    break;
               default:
                    throw std::logic_error( "unknown server type" );
          }
     }
}

namespace
{

struct UnitIdRangeComparator
{
     bool operator()( const UnitIdRange& lhs, const UnitIdRange& rhs ) const
     {
          return lhs.end < rhs.begin;
     }
};

struct UnitIdRangeValidator
{
     std::set< UnitIdRange, UnitIdRangeComparator > unitIdSet;
     bool defaultUnitIdsClient = false;
     modbus::UnitId unitIdCounter = 0;

     void Insert( TraceDeep& traceDeep, const std::vector< UnitIdRange >& unitIdRanges )
     {

          if( unitIdRanges.empty() )
          {
               if( defaultUnitIdsClient )
               {
                    std::stringstream ss;
                    ss << "default unit id range already exist on other client";
                    throw InvalidValueException( traceDeep, ss.str() );
               }
               else
               {
                    defaultUnitIdsClient = true;
               }
          }
          for( const auto& unitIdRange: unitIdRanges )
          {
               auto res = unitIdSet.insert( unitIdRange );
               if( !res.second )
               {
                    std::stringstream ss;
                    ss << "unit id range " << unitIdRange.begin << "-" << unitIdRange.end
                       << " intersected with unit id range on other client";
                    throw InvalidValueException( traceDeep, ss.str() );
               }
               if( unitIdRange.begin == unitIdRange.end )
               {
                    ++unitIdCounter;
               }
               else
               {
                    unitIdCounter += ( unitIdRange.end - unitIdRange.begin );
               }
          }
     }

     void CheckCoverage( TraceDeep& traceDeep ) const
     {
          if( unitIdCounter < ( modbus::unitIdMax - modbus::unitIdMin ) && !defaultUnitIdsClient )
          {
               std::stringstream ss;
               ss << "unit id for all clients not coverage all supported unit id (" << modbus::unitIdMin << "-"
                  << modbus::unitIdMax << "), set default client";
               throw InvalidValueException( traceDeep, ss.str() );
          }
     }
};

}

void ValidateClients( TracePath& tracePath, const std::vector< TransportConfigPtr >& clients )
{
     using AddressSet = std::vector< asio::ip::address >;
     std::map< asio::ip::port_type, AddressSet > tcpClientEnpoints;
     UnitIdRangeValidator unitIdRangeValidator;

     TraceDeep td( tracePath, "clients" );

     size_t index = 0;
     for( const auto& client: clients )
     {
          TraceDeep tdClient( td.GetTracePath(), std::to_string( index ) );
          switch( client->GetType() )
          {
               case TCP:
               {
                    const auto& clientPtr = std::dynamic_pointer_cast< TcpClientConfig >( client );
                    if( !clientPtr )
                    {
                         throw std::runtime_error( "cast to TcpClientConfig failed" );
                    }
                    if( clientPtr->GetAddress() == asio::ip::address_v4::any() )
                    {
                         std::stringstream ss;
                         ss << "any ip address don't allow for client";
                         throw InvalidValueException( tdClient, ss.str() );
                    }

                    auto it = tcpClientEnpoints.find( clientPtr->GetPort() );
                    if( it == tcpClientEnpoints.end() )
                    {
                         tcpClientEnpoints[ clientPtr->GetPort() ] = AddressSet { clientPtr->GetAddress() };
                         unitIdRangeValidator.Insert( tdClient, clientPtr->GetUnitIds() );
                    }
                    else
                    {
                         AddressSet& addressSet = it->second;

                         const auto addrIt = std::find( addressSet.begin(), addressSet.end(), clientPtr->GetAddress() );
                         if( addrIt != addressSet.end() )
                         {
                              std::stringstream ss;
                              ss << "ip address " << clientPtr->GetAddress().to_string()
                                 << " intersected with ip address " << addrIt->to_string() << " on other client";
                              throw InvalidValueException( tdClient, ss.str() );
                         }

                         addressSet.push_back( clientPtr->GetAddress() );
                         unitIdRangeValidator.Insert( tdClient, clientPtr->GetUnitIds() );
                    }
               }
                    break;
               default:
                    throw std::logic_error( "unknown server type" );
          }
     }

     unitIdRangeValidator.CheckCoverage( td );
}

}