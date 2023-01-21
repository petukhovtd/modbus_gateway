#ifndef MODBUS_GATEWAY_CONVERTORS_H
#define MODBUS_GATEWAY_CONVERTORS_H

#include <types.h>
#include <common/fmt_logger.h>
#include <config/config_types.h>

#include <string>
#include <optional>

namespace modbus_gateway
{

std::optional< FmtLogger::LogLevel > ConvertLogLevel( const std::string& logLevel );

std::optional< TransportType > ConvertTransportType( const std::string& transportType );

std::optional< NumericRangeType > ConvertNumericRangeType( const std::string& numericRangeType );

}

#endif
