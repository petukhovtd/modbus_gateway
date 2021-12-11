#include <gtest/gtest.h>
#include <modbus/modbus_buffer_tcp_wrapper.h>

using namespace modbus;

TEST( ModbusBufferTcpWrapper, Create )
{
     {
          ModbusBuffer modbusBuffer( FrameType::TCP );
          EXPECT_NO_THROW( ModbusBufferTcpWrapper modbusBufferTcpWrapper( modbusBuffer ) );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::RTU );
          EXPECT_THROW( ModbusBufferTcpWrapper modbusBufferTcpWrapper( modbusBuffer ), std::exception );
     }
     {
          ModbusBuffer modbusBuffer( FrameType::ASCII );
          EXPECT_THROW( ModbusBufferTcpWrapper modbusBufferTcpWrapper( modbusBuffer ), std::exception );
     }
}

TEST( ModbusBufferTcpWrapper, TransactionId )
{
     static const uint16_t transactionId = 0xA5A5;
     ModbusBuffer modbusBuffer( FrameType::TCP );
     ModbusBufferTcpWrapper modbusBufferTcpWrapper( modbusBuffer );
     modbusBufferTcpWrapper.SetTransactionId( transactionId );
     EXPECT_EQ( modbusBufferTcpWrapper.GetTransactionId(), transactionId );
}

TEST( ModbusBufferTcpWrapper, ProtocolId )
{
     static const uint16_t protocolId = 0xA5A5;
     ModbusBuffer modbusBuffer( FrameType::TCP );
     ModbusBufferTcpWrapper modbusBufferTcpWrapper( modbusBuffer );
     modbusBufferTcpWrapper.SetProtocolId( protocolId );
     EXPECT_EQ( modbusBufferTcpWrapper.GetProtocolId(), protocolId );
}

TEST( ModbusBufferTcpWrapper, LengthId )
{
     static const uint16_t LengthId = 0xA5A5;
     ModbusBuffer modbusBuffer( FrameType::TCP );
     ModbusBufferTcpWrapper modbusBufferTcpWrapper( modbusBuffer );
     modbusBufferTcpWrapper.SetLength( LengthId );
     EXPECT_EQ( modbusBufferTcpWrapper.GetLength(), LengthId );
}
