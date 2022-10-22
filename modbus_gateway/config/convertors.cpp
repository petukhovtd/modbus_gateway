#include "convertors.h"

namespace modbus_gateway
{

std::optional< FmtLogger::LogLevel > ConvertLogLevel( const std::string& logLevel )
{
     if( "trace" == logLevel )
     {
          return FmtLogger::LogLevel::Trace;
     }
     if( "debug" == logLevel )
     {
          return FmtLogger::LogLevel::Debug;
     }
     if( "info" == logLevel )
     {
          return FmtLogger::LogLevel::Info;
     }
     if( "warning" == logLevel )
     {
          return FmtLogger::LogLevel::Warn;
     }
     if( "error" == logLevel )
     {
          return FmtLogger::LogLevel::Error;
     }
     if( "critical" == logLevel )
     {
          return FmtLogger::LogLevel::Crit;
     }
     return std::nullopt;
}

std::optional< TransportType > ConvertTransportType( const std::string& transportType )
{
     if( "tcp" == transportType )
     {
          return TransportType::TCP;
     }
     return std::nullopt;
}

std::optional< NumericRangeType > ConvertNumericRangeType( const std::string& numericRangeType )
{
     if( "range" == numericRangeType )
     {
          return NumericRangeType::Range;
     }
     if( "value" == numericRangeType )
     {
          return NumericRangeType::Value;
     }
     return std::nullopt;
}

}
