#include <gtest/gtest.h>
#include <modbus/modbus_buffer_rtu_wrapper.h>
#include <modbus_test_common.h>

using namespace modbus;

TEST( ModbusBufferRtuWrapper, Create )
{
     {
          ModbusBuffer modbusBuffer( FrameType::TCP );
          EXPECT_THROW( ModbusBufferRtuWrapper modbusBufferRtuWrapper( modbusBuffer ), std::exception );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::RTU );
          EXPECT_NO_THROW( ModbusBufferRtuWrapper modbusBufferRtuWrapper( modbusBuffer ) );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::ASCII );
          EXPECT_THROW( ModbusBufferRtuWrapper modbusBufferRtuWrapper( modbusBuffer ), std::exception );
     }
}

TEST( ModbusBufferRtuWrapper, Check )
{
     {
          static const AduBuffer rtuFrame = { 0x1, 0x6, 0xDF, 0x62, 0x38 };
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( rtuFrame, FrameType::RTU );
          EXPECT_EQ( ModbusBufferRtuWrapper( modbusBuffer ).Check(), CheckFrameResult::NoError );
     }
     {
          static const AduBuffer rtuFrame = { 0x1, 0x6, 0xDF, 0x12, 0x34 };
          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( rtuFrame, FrameType::RTU );
          EXPECT_EQ( ModbusBufferRtuWrapper( modbusBuffer ).Check(), CheckFrameResult::RtuInvalidCrc );
     }
}

TEST( ModbusBufferRtuWrapper, Update )
{
     static const AduBuffer rtuFrame = { 0x1, 0x6, 0xDF, 0, 0 };
     static const AduBuffer newRtuFrame = { 0x1, 0x6, 0xDF, 0x62, 0x38 };

     ModbusBuffer modbusBuffer( FrameType::RTU );
     std::copy( rtuFrame.begin(), rtuFrame.end(), modbusBuffer.begin() );
     modbusBuffer.SetAduSize( rtuFrame.size() );

     ModbusBufferRtuWrapper( modbusBuffer ).Update();
     EXPECT_TRUE( test::Compare( newRtuFrame.begin(), newRtuFrame.end(),
                                 modbusBuffer.cbegin(), modbusBuffer.cend() ) );
}
