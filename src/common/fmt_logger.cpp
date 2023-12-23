#include <common/fmt_logger.h>

namespace modbus_gateway::common
{

void FmtLogger::SetLogLevel( FmtLogger::LogLevel logLevel )
{
     spdlog::default_logger()->set_level( static_cast< spdlog::level::level_enum >( logLevel ) );
}

}
