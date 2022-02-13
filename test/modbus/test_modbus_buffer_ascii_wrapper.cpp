#include <gtest/gtest.h>
#include <modbus/modbus_buffer_ascii_wrapper.h>
#include <modbus_test_common.h>

using namespace modbus;

TEST( ModbusBufferAsciiWrapper, Create )
{
     {
          ModbusBuffer modbusBuffer( FrameType::TCP );
          EXPECT_THROW( ModbusBufferAsciiWrapper modbusBufferAsciiWrapper( modbusBuffer ), std::exception );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::RTU );
          EXPECT_THROW( ModbusBufferAsciiWrapper modbusBufferAsciiWrapper( modbusBuffer ), std::exception );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::ASCII );
          EXPECT_NO_THROW( ModbusBufferAsciiWrapper modbusBufferAsciiWrapper( modbusBuffer ) );
     }
}

TEST( ModbusBufferAsciiWrapper, UpdateLrc )
{
     static const AduBuffer asciiInput = { asciiStart, '0', '1', '0', 'A', 'B', 'C', 0, 0, asciiEnd >> 8u,
                                           asciiEnd & 0xFFu };
     static const AduBuffer asciiOutput = { asciiStart, '0', '1', '0', 'A', 'B', 'C', 'A', '9', asciiEnd >> 8u,
                                            asciiEnd & 0xFFu };
     ModbusBuffer modbusBuffer( FrameType::ASCII );
     std::copy( asciiInput.begin(), asciiInput.end(), modbusBuffer.begin() );
     modbusBuffer.SetAduSize( asciiInput.size() );

     ModbusBufferAsciiWrapper modbusBufferAsciiWrapper( modbusBuffer );
     modbusBufferAsciiWrapper.UpdateLrc();
     EXPECT_TRUE( test::Compare( asciiOutput.begin(), asciiOutput.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
}

