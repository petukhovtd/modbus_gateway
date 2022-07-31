#include <gtest/gtest.h>
#include <test/modbus_common.h>
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
     static const AduBuffer tcpFrame = { 0x12, 0x34, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4 };
     static const TransactionId originId = 0x1234;

     static const AduBuffer newTcpFrame = { 0x56, 0x78, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4 };
     static const TransactionId newId = 0x5678;

     ModbusBuffer modbusBuffer = test::MakeModbusBuffer( tcpFrame, FrameType::TCP );

     ModbusBufferTcpWrapper modbusBufferTcpWrapper( modbusBuffer );

     EXPECT_EQ( originId, modbusBufferTcpWrapper.GetTransactionId() );

     modbusBufferTcpWrapper.SetTransactionId( newId );
     EXPECT_TRUE( test::Compare( newTcpFrame.cbegin(), newTcpFrame.cend(),
                                 modbusBuffer.cbegin(), modbusBuffer.cend() ) );
}

TEST( ModbusBufferTcpWrapper, ProtocolId )
{
     static const AduBuffer tcpFrame = { 0x12, 0x34, 0x9A, 0xBD, 0x0, 0x3, 0x1, 0x3, 0x4 };
     static const ProtocolId originProtocolId = 0x9ABD;

     ModbusBuffer modbusBuffer = test::MakeModbusBuffer( tcpFrame, FrameType::TCP );

     ModbusBufferTcpWrapper modbusBufferTcpWrapper( modbusBuffer );
     EXPECT_EQ( modbusBufferTcpWrapper.GetProtocolId(), originProtocolId );
}

TEST( ModbusBufferTcpWrapper, LengthId )
{
     static const AduBuffer tcpFrame = { 0x12, 0x34, 0x9A, 0xBD, 0x0, 0x3, 0x1, 0x3, 0x4 };
     static const Length originLength = 0x3;

     ModbusBuffer modbusBuffer = test::MakeModbusBuffer( tcpFrame, FrameType::TCP );

     ModbusBufferTcpWrapper modbusBufferTcpWrapper( modbusBuffer );
     EXPECT_EQ( modbusBufferTcpWrapper.GetLength(), originLength );
}

TEST( ModbusBufferTcpWrapper, Check )
{
     {
          static const AduBuffer tcpFrame = { 0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4 };

          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( tcpFrame, FrameType::TCP );

          EXPECT_EQ( ModbusBufferTcpWrapper( modbusBuffer ).Check(), CheckFrameResult::NoError );
     }
     {
          static const AduBuffer tcpFrame = { 0x0, 0x1, 0x0, 0x1, 0x0, 0x3, 0x1, 0x3, 0x4 };

          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( tcpFrame, FrameType::TCP );

          EXPECT_EQ( ModbusBufferTcpWrapper( modbusBuffer ).Check(), CheckFrameResult::TcpInvalidProtocolId );
     }
     {
          static const AduBuffer tcpFrame = { 0x0, 0x1, 0x0, 0x0, 0x0, 0x2, 0x1, 0x3, 0x4 };

          ModbusBuffer modbusBuffer = test::MakeModbusBuffer( tcpFrame, FrameType::TCP );

          EXPECT_EQ( ModbusBufferTcpWrapper( modbusBuffer ).Check(), CheckFrameResult::TcpInvalidLength );
     }
     {
          static const AduBuffer tcpFrame = { 0x0, 0x1, 0x0, 0x0, 0x0, 0x10, 0x1, 0x3, 0x4 };

          ModbusBuffer modbusBuffer( FrameType::TCP );
          std::copy( tcpFrame.begin(), tcpFrame.end(), modbusBuffer.begin() );
          modbusBuffer.SetAduSize( tcpFrame.size() );

          EXPECT_EQ( ModbusBufferTcpWrapper( modbusBuffer ).Check(), CheckFrameResult::TcpInvalidLength );
     }
}

TEST( ModbusBufferTcpWrapper, Update )
{
     static const AduBuffer tcpFrame = { 0x0, 0x1, 0x12, 0x34, 0x56, 0x48, 0x1, 0x3, 0x4 };
     static const AduBuffer newTcpFrame = { 0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4 };

     ModbusBuffer modbusBuffer = test::MakeModbusBuffer( tcpFrame, FrameType::TCP );
     ModbusBufferTcpWrapper( modbusBuffer ).Update();

     EXPECT_TRUE( test::Compare( newTcpFrame.cbegin(), newTcpFrame.cend(),
                                 modbusBuffer.cbegin(), modbusBuffer.cend() ) );
}