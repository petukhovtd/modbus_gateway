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

TEST( ModbusBufferAsciiWrapper, Check )
{
     {
          static const AduBuffer asciiFrame = { asciiStart,
                                                '0', '1', '0', 'A', 'B', 'C', 'A', '9',
                                                asciiEnd >> 8u, asciiEnd & 0xFFu };
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( asciiFrame, FrameType::ASCII );
          EXPECT_EQ( ModbusBufferAsciiWrapper( modbusBuffer ).Check(), CheckFrameResult::NoError );
     }
     {
          static const AduBuffer asciiFrame = { asciiStart / 2,
                                                '0', '1', '0', 'A', 'B', 'C', 'A', '9',
                                                asciiEnd >> 8u, asciiEnd & 0xFFu };
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( asciiFrame, FrameType::ASCII );
          EXPECT_EQ( ModbusBufferAsciiWrapper( modbusBuffer ).Check(), CheckFrameResult::AsciiInvalidStartTag );
     }
     {
          static const AduBuffer asciiFrame = { asciiStart,
                                                '0', '1', '0', 'A', 'B', 'C', 'A', '9',
                                                asciiEnd >> 7u, asciiEnd & 0xFFu };
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( asciiFrame, FrameType::ASCII );
          EXPECT_EQ( ModbusBufferAsciiWrapper( modbusBuffer ).Check(), CheckFrameResult::AsciiInvalidEndTag );
     }
     {
          static const AduBuffer asciiFrame = { asciiStart,
                                                '0', '1', '0', 'A', 'B', 'C', 'A', '9',
                                                asciiEnd >> 8u, asciiEnd & 0xF0u };
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( asciiFrame, FrameType::ASCII );
          EXPECT_EQ( ModbusBufferAsciiWrapper( modbusBuffer ).Check(), CheckFrameResult::AsciiInvalidEndTag );
     }
     {
          static const AduBuffer asciiFrame = { asciiStart,
                                                '0', '1', '0', 'A', 'B', 'C', 0, 0,
                                                asciiEnd >> 8u, asciiEnd & 0xFFu };
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( asciiFrame, FrameType::ASCII );
          EXPECT_EQ( ModbusBufferAsciiWrapper( modbusBuffer ).Check(), CheckFrameResult::AsciiInvalidLrc );
     }
}

TEST( ModbusBufferAsciiWrapper, Update )
{
     static const AduBuffer asciiFrame = { 0,
                                           '0', '1', '0', 'A', 'B', 'C', 0, 0,
                                           0, 0 };
     static const AduBuffer newAsciiFrame = { asciiStart,
                                              '0', '1', '0', 'A', 'B', 'C', 'A', '9',
                                              asciiEnd >> 8u, asciiEnd & 0xFFu };

     ModbusBuffer modbusBuffer = test::MakeModbusBuffer( asciiFrame, FrameType::ASCII );

     ModbusBufferAsciiWrapper modbusBufferAsciiWrapper( modbusBuffer );
     modbusBufferAsciiWrapper.Update();
     EXPECT_TRUE( test::Compare( newAsciiFrame.cbegin(), newAsciiFrame.cend(),
                                 modbusBuffer.cbegin(), modbusBuffer.cend() ) );
}

