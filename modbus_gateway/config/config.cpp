#include "config.h"
#include "trace_path.h"
#include "trace_deep.h"
#include "extractor.h"
#include "tcp_server_config.h"
#include "validators.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace modbus_gateway
{

Config::Config( std::istream& in )
: data_( json::parse( in ) )
{
     TracePath tracePath;
     tracePath.Push( "root" );

     {
          const auto& res = ExtractLogLevel( tracePath, data_ );
          if( res.has_value() )
          {
               logLevel_ = res.value();
          }
          else
          {
               logLevel_ = FmtLogger::LogLevel::Info;
          }
     }
     servers_ = ExtractServers( tracePath, data_ );
     clients_ = ExtractClients( tracePath, data_ );
}

FmtLogger::LogLevel Config::GetLogLevel() const
{
     return logLevel_;
}

const std::vector< TransportConfigPtr >& Config::GetServers() const
{
     return servers_;
}

const std::vector< TransportConfigPtr >& Config::GetClients() const
{
     return clients_;
}

void Config::Validate() const
{
     TracePath tracePath;
     tracePath.Push( "root" );

     ValidateServers( tracePath, servers_ );
     ValidateClients( tracePath, clients_ );
}

}
