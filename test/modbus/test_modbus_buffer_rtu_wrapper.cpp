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

TEST( ModbusBufferRtuWrapper, UpdateLrc )
{
     static const AduBuffer rtuInput = { 0x1, 0x6, 0xDF, 0, 0 };
     static const AduBuffer rtuOutput = { 0x1, 0x6, 0xDF, 0x62, 0x38 };
     ModbusBuffer modbusBuffer( FrameType::RTU );
     std::copy( rtuInput.begin(), rtuInput.end(), modbusBuffer.begin() );
     modbusBuffer.SetAduSize( rtuInput.size() );

     ModbusBufferRtuWrapper modbusBufferRtuWrapper( modbusBuffer );
     modbusBufferRtuWrapper.UpdateCrc();
     EXPECT_TRUE( test::Compare( rtuOutput.begin(), rtuOutput.end(), modbusBuffer.cbegin(), modbusBuffer.cend() ) );
}
