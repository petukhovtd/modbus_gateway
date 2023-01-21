#ifndef MODBUS_GATEWAY_FMT_LOGGER_H
#define MODBUS_GATEWAY_FMT_LOGGER_H

#include <spdlog/spdlog.h>

#define FMT_LOG_TRACE( format, ... ) spdlog::trace( format, ##__VA_ARGS__ );
#define FMT_LOG_DEBUG( format, ... ) spdlog::debug( format, ##__VA_ARGS__ );
#define FMT_LOG_INFO( format, ... ) spdlog::info( format, ##__VA_ARGS__ );
#define FMT_LOG_WARN( format, ... ) spdlog::warn( format, ##__VA_ARGS__ );
#define FMT_LOG_ERROR( format, ... ) spdlog::error( format, ##__VA_ARGS__ );
#define FMT_LOG_CRIT( format, ... ) spdlog::critical( format, ##__VA_ARGS__ );

namespace modbus_gateway
{

class FmtLogger
{
public:
     enum class LogLevel
     {
          Trace = spdlog::level::trace,
          Debug = spdlog::level::debug,
          Info = spdlog::level::info,
          Warn = spdlog::level::warn,
          Error = spdlog::level::err,
          Crit = spdlog::level::critical,
     };

     static void SetLogLevel( LogLevel logLevel );
};

}

#endif
