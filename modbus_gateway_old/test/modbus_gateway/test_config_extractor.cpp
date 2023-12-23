#include <gtest/gtest.h>
#include <config/extractor.h>
#include <config/trace_path.h>
#include <config/trace_deep.h>
#include <config/key_not_found_exception.h>
#include <config/invalid_type_exception.h>
#include <config/keys.h>
#include <config/tcp_server_config.h>
#include <config/unit_id_range.h>
#include <config/tcp_client_config.h>

#include <nlohmann/json.hpp>


TEST( ConfigExtractorTest, FindObject )
{
     {
          static const std::string key = "target";
          static const std::string value = "value";
          nlohmann::json obj;
          obj[ key ] = value;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, key );
          nlohmann::json::value_type res;
          ASSERT_NO_THROW( res = modbus_gateway::FindObject( traceDeep, obj ) );
          EXPECT_EQ( res, value );
     }
     {
          static const std::string key = "target";
          nlohmann::json obj;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, key );
          EXPECT_THROW( modbus_gateway::FindObject( traceDeep, obj ), modbus_gateway::KeyNotFoundException );
     }
}

TEST( ConfigExtractorTest, CheckType )
{
     {
          nlohmann::json obj;
          obj[ "key" ] = "value";
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, "key" );
          EXPECT_NO_THROW( modbus_gateway::CheckType( traceDeep, obj, modbus_gateway::ValueType::Object ) );
     }
     {
          nlohmann::json obj = { 1, 2, 3 };
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, "key" );
          EXPECT_NO_THROW( modbus_gateway::CheckType( traceDeep, obj, modbus_gateway::ValueType::Array ) );
     }
     {
          nlohmann::json obj = std::string();
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, "key" );
          EXPECT_NO_THROW( modbus_gateway::CheckType( traceDeep, obj, modbus_gateway::ValueType::String ) );
     }
     {
          nlohmann::json obj = true;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, "key" );
          EXPECT_NO_THROW( modbus_gateway::CheckType( traceDeep, obj, modbus_gateway::ValueType::Boolean ) );
     }
     {
          nlohmann::json obj = false;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, "key" );
          EXPECT_NO_THROW( modbus_gateway::CheckType( traceDeep, obj, modbus_gateway::ValueType::Boolean ) );
     }
     {
          nlohmann::json obj = 3;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, "key" );
          EXPECT_NO_THROW( modbus_gateway::CheckType( traceDeep, obj, modbus_gateway::ValueType::Number ) );
     }
     {
          nlohmann::json obj = 3.0;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, "key" );
          EXPECT_NO_THROW( modbus_gateway::CheckType( traceDeep, obj, modbus_gateway::ValueType::Number ) );
     }
     {
          nlohmann::json obj;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, "key" );
          EXPECT_NO_THROW( modbus_gateway::CheckType( traceDeep, obj, modbus_gateway::ValueType::Unknown ) );
     }
     {
          nlohmann::json obj = nullptr;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, "key" );
          EXPECT_NO_THROW( modbus_gateway::CheckType( traceDeep, obj, modbus_gateway::ValueType::Unknown ) );
     }
     {
          nlohmann::json obj = 123;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, "key" );
          EXPECT_THROW( modbus_gateway::CheckType( traceDeep, obj, modbus_gateway::ValueType::String ),
                        modbus_gateway::InvalidTypeException );
     }
     {
          nlohmann::json obj = { 123 };
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TraceDeep traceDeep( tracePath, "key" );
          EXPECT_THROW( modbus_gateway::CheckType( traceDeep, obj, modbus_gateway::ValueType::Boolean ),
                        modbus_gateway::InvalidTypeException );
     }
}

TEST( ConfigExtractorTest, ExtractLogLevel )
{
     static const std::string& key = modbus_gateway::keys::logLevel;
     {
          nlohmann::json obj;
          obj[ key ] = "error";
          modbus_gateway::TracePath tracePath;
          std::optional< modbus_gateway::FmtLogger::LogLevel > result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractLogLevel( tracePath, obj ) );
          ASSERT_TRUE( result.has_value() );
          EXPECT_EQ( result.value(), modbus_gateway::FmtLogger::LogLevel::Error );
     }
     {
          nlohmann::json obj;
          obj[ "same_key" ] = "val";
          modbus_gateway::TracePath tracePath;
          std::optional< modbus_gateway::FmtLogger::LogLevel > result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractLogLevel( tracePath, obj ) );
          EXPECT_FALSE( result.has_value() );
     }
     {
          nlohmann::json obj;
          obj[ key ] = "many";
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractLogLevel( tracePath, obj ), modbus_gateway::InvalidValueException );
     }
     {
          nlohmann::json obj;
          obj[ key ] = 123;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractLogLevel( tracePath, obj ), modbus_gateway::InvalidTypeException );
     }
}

TEST( ConfigExtractorTest, ExtractIpAddress )
{
     static const std::string& key = modbus_gateway::keys::ipAddress;
     {
          nlohmann::json obj;
          obj[ key ] = "127.0.0.1";
          modbus_gateway::TracePath tracePath;
          asio::ip::address result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractIpAddress( tracePath, obj ) );
          EXPECT_EQ( result.to_string(), "127.0.0.1" );
     }
     {
          nlohmann::json obj;
          obj[ "same_key" ] = "val";
          modbus_gateway::TracePath tracePath;
          std::optional< asio::ip::address > result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractIpAddressOptional( tracePath, obj ) );
          EXPECT_FALSE( result.has_value() );
     }
     {
          nlohmann::json obj;
          obj[ key ] = 123;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractIpAddress( tracePath, obj ), modbus_gateway::InvalidTypeException );
          EXPECT_THROW( modbus_gateway::ExtractIpAddressOptional( tracePath, obj ), modbus_gateway::InvalidTypeException );
     }
     {
          nlohmann::json obj;
          obj[ key ] = "123.456.789.012";
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractIpAddress( tracePath, obj ), modbus_gateway::InvalidValueException );
          EXPECT_THROW( modbus_gateway::ExtractIpAddressOptional( tracePath, obj ), modbus_gateway::InvalidValueException );
     }

}

TEST( ConfigExtractorTest, ExtractIpPort )
{
     static const std::string& key = modbus_gateway::keys::ipPort;
     {
          nlohmann::json obj;
          obj[ key ] = 1234;
          modbus_gateway::TracePath tracePath;
          asio::ip::port_type result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractIpPort( tracePath, obj ) );
          EXPECT_EQ( result, 1234 );
     }
     {
          nlohmann::json obj;
          obj[ "same_key" ] = "val";
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractIpPort( tracePath, obj ), modbus_gateway::KeyNotFoundException );
     }
     {
          nlohmann::json obj;
          obj[ key ] = 0;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractIpPort( tracePath, obj ), modbus_gateway::InvalidValueException );

          obj[ key ] = 65555;
          EXPECT_THROW( modbus_gateway::ExtractIpPort( tracePath, obj ), modbus_gateway::InvalidValueException );

          obj[ key ] = -1;
          EXPECT_THROW( modbus_gateway::ExtractIpPort( tracePath, obj ), modbus_gateway::InvalidValueException );
     }
     {
          nlohmann::json obj;
          obj[ key ] = "0";
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractIpPort( tracePath, obj ), modbus_gateway::InvalidTypeException );
     }
}

TEST( ConfigExtractorTest, ExtractTransportType )
{
     static const std::string& key = modbus_gateway::keys::transportType;
     {
          nlohmann::json obj;
          obj[ key ] = "tcp";
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TransportType result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractTransportType( tracePath, obj ) );
          EXPECT_EQ( result, modbus_gateway::TransportType::TCP );
     }
     {
          nlohmann::json obj;
          obj[ "same_key" ] = "val";
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractTransportType( tracePath, obj ), modbus_gateway::KeyNotFoundException );
     }
     {
          nlohmann::json obj;
          obj[ key ] = 123;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractTransportType( tracePath, obj ), modbus_gateway::InvalidTypeException );
     }
     {
          nlohmann::json obj;
          obj[ key ] = "123";
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractTransportType( tracePath, obj ), modbus_gateway::InvalidValueException );
     }
}

TEST( ConfigExtractorTest, ExtractServer )
{
     static const std::string& typeKey = modbus_gateway::keys::transportType;
     static const std::string& addrKey = modbus_gateway::keys::ipAddress;
     static const std::string& portKey = modbus_gateway::keys::ipPort;
     {
          nlohmann::json obj;
          obj[ typeKey ] = "tcp";
          obj[ addrKey ] = "127.0.0.1";
          obj[ portKey ] = 502;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TransportConfigPtr result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractServer( tracePath, obj ) );
          ASSERT_EQ( result->GetType(), modbus_gateway::TransportType::TCP );
          const std::shared_ptr< modbus_gateway::TcpServerConfig >& ptr = std::dynamic_pointer_cast< modbus_gateway::TcpServerConfig >( result );
          ASSERT_TRUE( ptr );
          EXPECT_EQ( ptr->GetAddress().to_string(), "127.0.0.1" );
          EXPECT_EQ( ptr->GetPort(), 502 );
     }
     {
          nlohmann::json obj;
          obj[ typeKey ] = "tcp";
          obj[ portKey ] = 502;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TransportConfigPtr result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractServer( tracePath, obj ) );
          ASSERT_EQ( result->GetType(), modbus_gateway::TransportType::TCP );
          const auto& ptr = std::dynamic_pointer_cast< modbus_gateway::TcpServerConfig >( result );
          ASSERT_TRUE( ptr );
          EXPECT_EQ( ptr->GetAddress().to_string(), "0.0.0.0" );
          EXPECT_EQ( ptr->GetPort(), 502 );
     }
     {
          nlohmann::json obj;
          obj[ typeKey ] = "tcp";
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TransportConfigPtr result;
          EXPECT_THROW( result = modbus_gateway::ExtractServer( tracePath, obj ), modbus_gateway::KeyNotFoundException );
     }
     {
          nlohmann::json obj;
          obj[ portKey ] = 502;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TransportConfigPtr result;
          EXPECT_THROW( result = modbus_gateway::ExtractServer( tracePath, obj ), modbus_gateway::KeyNotFoundException );
     }
}

TEST( ConfigExtractorTest, ExtractServers )
{
     static const std::string& serversKey = modbus_gateway::keys::servers;
     static const std::string& typeKey = modbus_gateway::keys::transportType;
     static const std::string& addrKey = modbus_gateway::keys::ipAddress;
     static const std::string& portKey = modbus_gateway::keys::ipPort;
     {
          nlohmann::json obj;
          obj[ serversKey ] = {
          { { typeKey, "tcp"}, { portKey, 502 } },
          { { typeKey, "tcp"}, { addrKey, "127.0.0.1" }, { portKey, 503 } }
          };
          modbus_gateway::TracePath tracePath;
          std::vector< modbus_gateway::TransportConfigPtr > result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractServers( tracePath, obj ) );
          ASSERT_EQ( result.size(), 2 );
          ASSERT_EQ( result[ 0 ]->GetType(), modbus_gateway::TransportType::TCP );
          ASSERT_EQ( result[ 1 ]->GetType(), modbus_gateway::TransportType::TCP );
     }
     {
          nlohmann::json obj;
          obj[ serversKey ] = 123;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractServers( tracePath, obj ), modbus_gateway::InvalidTypeException );
     }
     {
          nlohmann::json obj;
          obj[ serversKey ] = nlohmann::json::array();
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractServers( tracePath, obj ), modbus_gateway::InvalidValueException );
     }
     {
          nlohmann::json obj;
          obj[ "any_key" ] = {
          { { typeKey, "tcp"}, { portKey, 502 } },
          };
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractServers( tracePath, obj ), modbus_gateway::KeyNotFoundException );
     }
}

TEST( ConfigExtractorTest, ExtractModbusUnitId )
{
     static const std::string key = "key";
     {
          nlohmann::json obj;
          obj[ key ] = 1;
          modbus_gateway::TracePath tracePath;
          modbus::UnitId result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractModbusUnitId( tracePath, obj, key ) );
          EXPECT_EQ( result, 1 );
     }
     {
          nlohmann::json obj;
          obj[ key ] = modbus::unitIdMin - 1;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractModbusUnitId( tracePath, obj, key ), modbus_gateway::InvalidValueException );
     }
     {
          nlohmann::json obj;
          obj[ key ] = modbus::unitIdMax + 1;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractModbusUnitId( tracePath, obj, key ), modbus_gateway::InvalidValueException );
     }
     {
          nlohmann::json obj;
          obj[ key ] = "123";
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractModbusUnitId( tracePath, obj, key ), modbus_gateway::InvalidTypeException );
     }
     {
          nlohmann::json obj;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractModbusUnitId( tracePath, obj, key ), modbus_gateway::KeyNotFoundException );
     }
}

TEST( ConfigExtractorTest, ExtractNumber )
{
     static const std::string& key = "key";
     {
          nlohmann::json obj;
          obj[ key ] = 1;
          modbus_gateway::TracePath tracePath;
          uint8_t result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractUnsignedNumber< uint8_t >( tracePath, obj, key ) );
          EXPECT_EQ( result, 1 );
     }
     {
          nlohmann::json obj;
          obj[ key ] = 300;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractUnsignedNumber< uint8_t >( tracePath, obj, key ), modbus_gateway::InvalidValueException );
     }
     {
          nlohmann::json obj;
          obj[ "some_key" ] = 300;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractUnsignedNumber< uint8_t >( tracePath, obj, key ), modbus_gateway::KeyNotFoundException );
     }
     {
          nlohmann::json obj;
          obj[ key ] = "300";
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractUnsignedNumber< uint8_t >( tracePath, obj, key ), modbus_gateway::InvalidTypeException );
     }
}

TEST( ConfigExtractorTest, ExtractNumericRangeType )
{
     static const std::string& key = modbus_gateway::keys::numericRangeType;
     {
          nlohmann::json obj;
          obj[ key ] = "range";
          modbus_gateway::TracePath tracePath;
          modbus_gateway::NumericRangeType result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractNumericRangeType( tracePath, obj ) );
          EXPECT_EQ( result, modbus_gateway::NumericRangeType::Range );
     }
     {
          nlohmann::json obj;
          obj[ "same_key" ] = "val";
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractNumericRangeType( tracePath, obj ), modbus_gateway::KeyNotFoundException );
     }
     {
          nlohmann::json obj;
          obj[ key ] = 123;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractNumericRangeType( tracePath, obj ), modbus_gateway::InvalidTypeException );
     }
     {
          nlohmann::json obj;
          obj[ key ] = "123";
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractNumericRangeType( tracePath, obj ), modbus_gateway::InvalidValueException );
     }
}

TEST( ConfigExtractorTest, ExtractUnitId )
{
     static const std::string& type = modbus_gateway::keys::numericRangeType;
     static const std::string& value = modbus_gateway::keys::numericRangeValue;
     static const std::string& begin = modbus_gateway::keys::numericRangeBegin;
     static const std::string& end = modbus_gateway::keys::numericRangeEnd;
     {
          nlohmann::json obj;
          obj[ type ] = "value";
          obj[ value ] = 123;
          modbus_gateway::TracePath tracePath;
          std::optional< modbus_gateway::UnitIdRange > result;
          ASSERT_NO_THROW( result = modbus_gateway::UnitIdRange( tracePath, obj ) );
          ASSERT_TRUE( result );
          EXPECT_EQ( result->begin, result->end );
          EXPECT_EQ( result->begin, 123 );
     }
     {
          nlohmann::json obj;
          obj[ type ] = "range";
          obj[ begin ] = 5;
          obj[ end ] = 10;
          modbus_gateway::TracePath tracePath;
          std::optional< modbus_gateway::UnitIdRange > result;
          ASSERT_NO_THROW( result = modbus_gateway::UnitIdRange( tracePath, obj ) );
          ASSERT_TRUE( result );
          EXPECT_EQ( result->begin, 5 );
          EXPECT_EQ( result->end, 10 );
     }
     {
          nlohmann::json obj;
          obj[ type ] = "value";
          obj[ begin ] = 123;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::UnitIdRange( tracePath, obj ), modbus_gateway::KeyNotFoundException );
     }
     {
          nlohmann::json obj;
          obj[ type ] = "range";
          obj[ value ] = 123;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::UnitIdRange( tracePath, obj ), modbus_gateway::KeyNotFoundException );
     }
}

TEST( ConfigExtractorTest, ExtractClient )
{
     static const std::string& typeKey = modbus_gateway::keys::transportType;
     static const std::string& addrKey = modbus_gateway::keys::ipAddress;
     static const std::string& portKey = modbus_gateway::keys::ipPort;
     static const std::string& unitIdKey = modbus_gateway::keys::unitIdSet;
     static const std::string& idTypeKey = modbus_gateway::keys::numericRangeType;
     static const std::string& idValueKey = modbus_gateway::keys::numericRangeValue;
     static const std::string& idBeginKey = modbus_gateway::keys::numericRangeBegin;
     static const std::string& idEndKey = modbus_gateway::keys::numericRangeEnd;
     {
          nlohmann::json obj;
          obj[ typeKey ] = "tcp";
          obj[ addrKey ] = "127.0.0.1";
          obj[ portKey ] = 502;
          obj[ unitIdKey ] = {
          { { idTypeKey, "value" }, { idValueKey, 1 } }
          };
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TransportConfigPtr result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractClient( tracePath, obj ) );
          ASSERT_EQ( result->GetType(), modbus_gateway::TransportType::TCP );
          const auto& ptr = std::dynamic_pointer_cast< modbus_gateway::TcpClientConfig >( result );
          ASSERT_TRUE( ptr );
          EXPECT_EQ( ptr->GetAddress().to_string(), "127.0.0.1" );
          EXPECT_EQ( ptr->GetPort(), 502 );
          EXPECT_EQ( ptr->GetUnitIds().size(), 1 );
     }
     {
          nlohmann::json obj;
          obj[ typeKey ] = "tcp";
          obj[ addrKey ] = "127.0.0.1";
          obj[ portKey ] = 502;
          modbus_gateway::TracePath tracePath;
          modbus_gateway::TransportConfigPtr result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractClient( tracePath, obj ) );
          ASSERT_EQ( result->GetType(), modbus_gateway::TransportType::TCP );
          const auto& ptr = std::dynamic_pointer_cast< modbus_gateway::TcpClientConfig >( result );
          ASSERT_TRUE( ptr );
          EXPECT_EQ( ptr->GetAddress().to_string(), "127.0.0.1" );
          EXPECT_EQ( ptr->GetPort(), 502 );
          EXPECT_TRUE( ptr->GetUnitIds().empty() );
     }
     {
          nlohmann::json obj;
          obj[ typeKey ] = "tcp";
          obj[ addrKey ] = "127.0.0.1";
          obj[ portKey ] = 502;
          obj[ unitIdKey ] = nlohmann::json::array();
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractClient( tracePath, obj ), modbus_gateway::InvalidValueException );
     }
     {
          nlohmann::json obj;
          obj[ typeKey ] = "tcp";
          obj[ portKey ] = 502;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractClient( tracePath, obj ), modbus_gateway::KeyNotFoundException );
     }
     {
          nlohmann::json obj;
          obj[ typeKey ] = "tcp";
          obj[ addrKey ] = "127.0.0.1";
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractClient( tracePath, obj ), modbus_gateway::KeyNotFoundException );
     }
     {
          nlohmann::json obj;
          obj[ addrKey ] = "127.0.0.1";
          obj[ portKey ] = 502;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractClient( tracePath, obj ), modbus_gateway::KeyNotFoundException );
     }
}

TEST( ConfigExtractorTest, ExtractClients )
{
     static const std::string& clientsKey = modbus_gateway::keys::clients;
     static const std::string& typeKey = modbus_gateway::keys::transportType;
     static const std::string& addrKey = modbus_gateway::keys::ipAddress;
     static const std::string& portKey = modbus_gateway::keys::ipPort;
     {
          nlohmann::json obj;
          obj[ clientsKey ] = {
          { { typeKey, "tcp"}, { addrKey, "127.0.0.1" }, { portKey, 502 } },
          { { typeKey, "tcp"}, { addrKey, "127.0.0.1" }, { portKey, 503 } }
          };
          modbus_gateway::TracePath tracePath;
          std::vector< modbus_gateway::TransportConfigPtr > result;
          ASSERT_NO_THROW( result = modbus_gateway::ExtractClients( tracePath, obj ) );
          ASSERT_EQ( result.size(), 2 );
          ASSERT_EQ( result[ 0 ]->GetType(), modbus_gateway::TransportType::TCP );
          ASSERT_EQ( result[ 1 ]->GetType(), modbus_gateway::TransportType::TCP );
     }
     {
          nlohmann::json obj;
          obj[ clientsKey ] = 123;
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractClients( tracePath, obj ), modbus_gateway::InvalidTypeException );
     }
     {
          nlohmann::json obj;
          obj[ clientsKey ] = nlohmann::json::array();
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractClients( tracePath, obj ), modbus_gateway::InvalidValueException );
     }
     {
          nlohmann::json obj;
          obj[ "any_key" ] = {
          { { typeKey, "tcp"}, { addrKey, "127.0.0.1" }, { portKey, 502 } },
          };
          modbus_gateway::TracePath tracePath;
          EXPECT_THROW( modbus_gateway::ExtractClients( tracePath, obj ), modbus_gateway::KeyNotFoundException );
     }
}
