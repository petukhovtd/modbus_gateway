#include <gtest/gtest.h>
#include <modbus_test_common.h>
#include <modbus/modbus_buffer.h>
#include <modbus/modbus_types.h>

using namespace modbus;

TEST( ModbusBuffer, CreateBuffer )
{
     for( FrameType type: { FrameType::RTU, FrameType::TCP, FrameType::ASCII } )
     {
          ModbusBuffer modbusBuffer( type );
          EXPECT_EQ( modbusBuffer.GetType(), type );
          EXPECT_EQ( modbusBuffer.GetAduSize(), GetAduMaxSize( type ) );
          EXPECT_EQ( std::distance( modbusBuffer.begin(), modbusBuffer.end() ), modbusBuffer.GetAduSize() );
     }
}

TEST( ModbusBuffer, SetSize )
{
     static const size_t size = 100;
     for( FrameType type: { FrameType::RTU, FrameType::TCP, FrameType::ASCII } )
     {
          ModbusBuffer modbusBuffer( type );
          modbusBuffer.SetAduSize( size );
          EXPECT_EQ( std::distance( modbusBuffer.begin(), modbusBuffer.end() ), size );
     }
}

TEST( ModbusBuffer, WriteBuffer )
{
     static const std::vector< AduElementType > reference = { 1, 2, 3, 4, 5, 6, 7 };
     for( FrameType type: { FrameType::RTU, FrameType::TCP, FrameType::ASCII } )
     {
          ModbusBuffer modbusBuffer( type );
          std::copy( reference.begin(), reference.end(), modbusBuffer.begin() );
          modbusBuffer.SetAduSize( reference.size() );
          EXPECT_TRUE( test::Compare( reference.begin(), reference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
}

TEST( ModbusBuffer, UnitId )
{
     static const uint8_t unitId = 123;
     {
          ModbusBuffer modbusBuffer( FrameType::RTU );
          modbusBuffer.SetUnitId( unitId );
          EXPECT_EQ( modbusBuffer.GetUnitId(), unitId );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::TCP );
          modbusBuffer.SetUnitId( unitId );
          EXPECT_EQ( modbusBuffer.GetUnitId(), unitId );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::ASCII );
          modbusBuffer.SetUnitId( unitId );
          EXPECT_EQ( modbusBuffer.GetUnitId(), unitId );
     }
}

TEST( ModbusBuffer, FunctionCode )
{
     static const uint8_t functionCode = 55;
     {
          ModbusBuffer modbusBuffer( FrameType::RTU );
          modbusBuffer.SetFunctionCode( functionCode );
          EXPECT_EQ( modbusBuffer.GetFunctionCode(), functionCode );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::TCP );
          modbusBuffer.SetFunctionCode( functionCode );
          EXPECT_EQ( modbusBuffer.GetFunctionCode(), functionCode );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::ASCII );
          modbusBuffer.SetFunctionCode( functionCode );
          EXPECT_EQ( modbusBuffer.GetFunctionCode(), functionCode );
     }
}
