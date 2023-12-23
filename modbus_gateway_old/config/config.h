#ifndef MODBUS_GATEWAY_CONFIG_H
#define MODBUS_GATEWAY_CONFIG_H

#include <config/i_transport_config.h>
#include <common/fmt_logger.h>
#include <nlohmann/json.hpp>
#include <istream>

namespace modbus_gateway
{

class Config
{
public:
     explicit Config( std::istream& in );

     FmtLogger::LogLevel GetLogLevel() const;

     const std::vector< TransportConfigPtr >& GetServers() const;

     const std::vector< TransportConfigPtr >& GetClients() const;

     void Validate() const;

private:
     nlohmann::json data_;
     FmtLogger::LogLevel logLevel_;
     std::vector< TransportConfigPtr > servers_;
     std::vector< TransportConfigPtr > clients_;
};

}

#endif
