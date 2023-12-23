#include <gtest/gtest.h>
#include <config/convertors.h>

TEST( ConfigConvertorsTest, LogLevel )
{
     {
          const auto& res = modbus_gateway::ConvertLogLevel( "trace" );
          ASSERT_TRUE( res.has_value() );
          EXPECT_EQ( res.value(), modbus_gateway::FmtLogger::LogLevel::Trace );
     }
     {
          const auto& res = modbus_gateway::ConvertLogLevel( "debug" );
          ASSERT_TRUE( res.has_value() );
          EXPECT_EQ( res.value(), modbus_gateway::FmtLogger::LogLevel::Debug );
     }
     {
          const auto& res = modbus_gateway::ConvertLogLevel( "info" );
          ASSERT_TRUE( res.has_value() );
          EXPECT_EQ( res.value(), modbus_gateway::FmtLogger::LogLevel::Info );
     }
     {
          const auto& res = modbus_gateway::ConvertLogLevel( "warning" );
          ASSERT_TRUE( res.has_value() );
          EXPECT_EQ( res.value(), modbus_gateway::FmtLogger::LogLevel::Warn );
     }
     {
          const auto& res = modbus_gateway::ConvertLogLevel( "error" );
          ASSERT_TRUE( res.has_value() );
          EXPECT_EQ( res.value(), modbus_gateway::FmtLogger::LogLevel::Error );
     }
     {
          const auto& res = modbus_gateway::ConvertLogLevel( "critical" );
          ASSERT_TRUE( res.has_value() );
          EXPECT_EQ( res.value(), modbus_gateway::FmtLogger::LogLevel::Crit );
     }

     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "debugg" ).has_value() );
     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "Info" ).has_value() );
     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "INFO" ).has_value() );
     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "info " ).has_value() );
     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "" ).has_value() );
}

TEST( ConfigConvertorsTest, TransportType )
{
     {
          const auto& res = modbus_gateway::ConvertTransportType( "tcp" );
          ASSERT_TRUE( res.has_value() );
          EXPECT_EQ( res.value(), modbus_gateway::TransportType::TCP );
     }
     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "udp" ).has_value() );
     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "rtu" ).has_value() );
     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "mtu" ).has_value() );
     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "TCP" ).has_value() );
     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "" ).has_value() );
}

TEST( ConfigConvertorsTest, NumericRangeType )
{
     {
          const auto& res = modbus_gateway::ConvertNumericRangeType( "range" );
          ASSERT_TRUE( res.has_value() );
          EXPECT_EQ( res.value(), modbus_gateway::NumericRangeType::Range );
     }
     {
          const auto& res = modbus_gateway::ConvertNumericRangeType( "value" );
          ASSERT_TRUE( res.has_value() );
          EXPECT_EQ( res.value(), modbus_gateway::NumericRangeType::Value );
     }
     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "val" ).has_value() );
     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "RANGE" ).has_value() );
     EXPECT_FALSE( modbus_gateway::ConvertLogLevel( "" ).has_value() );
}