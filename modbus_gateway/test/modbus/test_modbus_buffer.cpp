#include <gtest/gtest.h>
#include <modbus_test_common.h>
#include <modbus/modbus_buffer.h>
#include <modbus/modbus_types.h>
#include <modbus/modbus_buffer_wrapper.h>

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
          EXPECT_TRUE( modbusBuffer.SetAduSize( GetAduMinSize( type ) ) );
          EXPECT_TRUE( modbusBuffer.SetAduSize( GetAduMaxSize( type ) ) );
          EXPECT_FALSE( modbusBuffer.SetAduSize( GetAduMinSize( type ) - 1 ) );
          EXPECT_FALSE( modbusBuffer.SetAduSize( GetAduMaxSize( type ) + 1 ) );
     }
}

TEST( ModbusBuffer, WriteBuffer )
{
     static const std::vector< AduElementType > reference = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
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

TEST( ModbusBuffer, ConvertToType )
{
     //                                                                    id   fc  data   crc
     static const AduBuffer rtuReference =                               { 0x1, 0x3, 0x4, 0x21, 0x33 };
     //                                       tr id    proto id  length
     static const AduBuffer tcpReference = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4 };
     static const AduBuffer asciiReference =                 { asciiStart, '0', '1', '0', '3', '0', '4', 'D', '8', asciiEnd >> 8u, asciiEnd & 0xFFu };

     {
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( rtuReference, FrameType::RTU );

          modbusBuffer.ConvertTo( FrameType::TCP );
          MakeModbusBufferWrapper( modbusBuffer )->Update();
          EXPECT_EQ( modbusBuffer.GetAduSize(), tcpReference.size() );
          EXPECT_TRUE( test::Compare( tcpReference.begin(), tcpReference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
     {
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( rtuReference, FrameType::RTU );

          modbusBuffer.ConvertTo( FrameType::ASCII );
          MakeModbusBufferWrapper( modbusBuffer )->Update();
          EXPECT_EQ( modbusBuffer.GetAduSize(), asciiReference.size() );
          EXPECT_TRUE( test::Compare( asciiReference.begin(), asciiReference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
     {
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( tcpReference, FrameType::TCP );

          modbusBuffer.ConvertTo( FrameType::RTU );
          MakeModbusBufferWrapper( modbusBuffer )->Update();
          EXPECT_EQ( modbusBuffer.GetAduSize(), rtuReference.size() );
          EXPECT_TRUE( test::Compare( rtuReference.begin(), rtuReference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
     {
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( tcpReference, FrameType::TCP );

          modbusBuffer.ConvertTo( FrameType::ASCII );
          MakeModbusBufferWrapper( modbusBuffer )->Update();
          EXPECT_EQ( modbusBuffer.GetAduSize(), asciiReference.size() );
          EXPECT_TRUE( test::Compare( asciiReference.begin(), asciiReference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
     {
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( asciiReference, FrameType::ASCII );

          modbusBuffer.ConvertTo( FrameType::RTU );
          MakeModbusBufferWrapper( modbusBuffer )->Update();
          EXPECT_EQ( modbusBuffer.GetAduSize(), rtuReference.size() );
          EXPECT_TRUE( test::Compare( rtuReference.begin(), rtuReference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
     {
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( asciiReference, FrameType::ASCII );

          modbusBuffer.ConvertTo( FrameType::TCP );
          MakeModbusBufferWrapper( modbusBuffer )->Update();
          EXPECT_EQ( modbusBuffer.GetAduSize(), tcpReference.size() );
          EXPECT_TRUE( test::Compare( tcpReference.begin(), tcpReference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
}
