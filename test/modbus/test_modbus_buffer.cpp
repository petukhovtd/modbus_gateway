#include <gtest/gtest.h>
#include <modbus_test_common.h>
#include <modbus/modbus_buffer.h>
#include <modbus/modbus_types.h>
#include <modbus/modbus_buffer_ascii_wrapper.h>
#include <modbus/modbus_buffer_rtu_wrapper.h>
#include <modbus/modbus_buffer_tcp_wrapper.h>

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

TEST( ModbusBuffer, ConvertToType )
{
     //                                                                    id   fc  data   crc
     static const AduBuffer rtuReference =                               { 0x1, 0x3, 0x4, 0x21, 0x33 };
     //                                       tr id    proto id  length
     static const AduBuffer tcpReference = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x3, 0x4 };
     static const AduBuffer asciiReference =                 { asciiStart, '0', '1', '0', '3', '0', '4', 'D', '8', asciiEnd >> 8u, asciiEnd & 0xFFu };

     {
          ModbusBuffer modbusBuffer( FrameType::RTU );
          std::copy( rtuReference.begin(), rtuReference.end(), modbusBuffer.begin() );
          modbusBuffer.SetAduSize( rtuReference.size() );

          modbusBuffer.ConvertTo( FrameType::TCP );
          EXPECT_EQ( modbusBuffer.GetAduSize(), tcpReference.size() );
          EXPECT_TRUE( test::Compare( tcpReference.begin(), tcpReference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::RTU );
          std::copy( rtuReference.begin(), rtuReference.end(), modbusBuffer.begin() );
          modbusBuffer.SetAduSize( rtuReference.size() );

          modbusBuffer.ConvertTo( FrameType::ASCII );
          ModbusBufferAsciiWrapper modbusBufferAsciiWrapper( modbusBuffer );
          modbusBufferAsciiWrapper.UpdateLrc();
          EXPECT_EQ( modbusBuffer.GetAduSize(), asciiReference.size() );
          EXPECT_TRUE( test::Compare( asciiReference.begin(), asciiReference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::TCP );
          std::copy( tcpReference.begin(), tcpReference.end(), modbusBuffer.begin() );
          modbusBuffer.SetAduSize( tcpReference.size() );

          modbusBuffer.ConvertTo( FrameType::RTU );
          ModbusBufferRtuWrapper modbusBufferRtuWrapper( modbusBuffer );
          modbusBufferRtuWrapper.UpdateCrc();
          EXPECT_EQ( modbusBuffer.GetAduSize(), rtuReference.size() );
          EXPECT_TRUE( test::Compare( rtuReference.begin(), rtuReference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::TCP );
          std::copy( tcpReference.begin(), tcpReference.end(), modbusBuffer.begin() );
          modbusBuffer.SetAduSize( tcpReference.size() );

          modbusBuffer.ConvertTo( FrameType::ASCII );
          ModbusBufferAsciiWrapper modbusBufferAsciiWrapper( modbusBuffer );
          modbusBufferAsciiWrapper.UpdateLrc();
          EXPECT_EQ( modbusBuffer.GetAduSize(), asciiReference.size() );
          EXPECT_TRUE( test::Compare( asciiReference.begin(), asciiReference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::ASCII );
          std::copy( asciiReference.begin(), asciiReference.end(), modbusBuffer.begin() );
          modbusBuffer.SetAduSize( asciiReference.size() );

          modbusBuffer.ConvertTo( FrameType::RTU );
          ModbusBufferRtuWrapper modbusBufferRtuWrapper( modbusBuffer );
          modbusBufferRtuWrapper.UpdateCrc();
          EXPECT_EQ( modbusBuffer.GetAduSize(), rtuReference.size() );
          EXPECT_TRUE( test::Compare( rtuReference.begin(), rtuReference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::ASCII );
          std::copy( asciiReference.begin(), asciiReference.end(), modbusBuffer.begin() );
          modbusBuffer.SetAduSize( asciiReference.size() );

          modbusBuffer.ConvertTo( FrameType::TCP );
          ModbusBufferTcpWrapper modbusBufferTcpWrapper( modbusBuffer );
          modbusBufferTcpWrapper.SetLength( 0 );
          EXPECT_EQ( modbusBuffer.GetAduSize(), tcpReference.size() );
          EXPECT_TRUE( test::Compare( tcpReference.begin(), tcpReference.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
     }
}
