#include <gtest/gtest.h>

#include <test/single_router.h>
#include <test/modbus_message_actor.h>
#include <test/modbus_common.h>
#include <test/modbus_tcp_client.h>

#include <exchange/exchange.h>
#include <types.h>
#include <messages/modbus_message.h>
#include <modbus_tcp_server.h>
#include <modbus/modbus_buffer_tcp_wrapper.h>

namespace
{

void
ExchangeMessage( const test::ModbusMessageActor::Handler& handler, const modbus_gateway::ModbusBufferPtr& sendBuffer,
                 const modbus_gateway::ModbusBufferPtr& receiveBuffer )
{
     const asio::ip::address_v4 addr = asio::ip::address_v4::loopback();
     const asio::ip::port_type port = 1234;

     modbus_gateway::FmtLogger::SetLogLevel( modbus_gateway::FmtLogger::LogLevel::Trace );

     modbus_gateway::ContextPtr context = std::make_shared< modbus_gateway::ContextPtr::element_type >();
     auto work = asio::executor_work_guard( context->get_executor() );
     auto contextThread = std::thread( [ context ]()
                                       {
                                            context->run();
                                       } );

     auto modbusEchoActor = test::ModbusMessageActor::Create( handler );

     const exchange::ActorId modbusEchoActorId = exchange::Exchange::Insert( modbusEchoActor );

     modbus_gateway::RouterPtr singleRouter = std::make_shared< test::SingleRouter >( modbusEchoActorId );

     auto tcpServer = modbus_gateway::ModbusTcpServer::Create( context, asio::ip::address_v4::loopback(), port,
                                                               singleRouter );

     exchange::Exchange::Insert( tcpServer );

     tcpServer->Start();

     test::ModbusTcpClient modbusTcpClient( context, asio::ip::address( asio::ip::address_v4::loopback() ), port );

     ASSERT_FALSE( modbusTcpClient.Connect() );

     modbusTcpClient.Process( sendBuffer, receiveBuffer );

     std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

     context->stop();
     if( contextThread.joinable() )
     {
          contextThread.join();
     }
}

}

TEST( ModbusTcpConnection, EchoTest )
{
     static const modbus::AduBuffer tcpFrame = { 0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4 };
     auto sendBuffer = std::make_shared< modbus::ModbusBuffer >(
     test::MakeModbusBuffer( tcpFrame, modbus::FrameType::TCP ) );
     auto receiveBuffer = std::make_shared< modbus::ModbusBuffer >( modbus::FrameType::TCP );

     ExchangeMessage( []( const modbus_gateway::ModbusMessagePtr& in )->modbus_gateway::ModbusMessagePtr
                      {
                           return in;
                      }, sendBuffer, receiveBuffer );
}

TEST( ModbusTcpConnection, SaveTransactionIdTest )
{
     static const modbus::AduBuffer tcpFrame = { 0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4 };
     auto sendBuffer = std::make_shared< modbus::ModbusBuffer >(
     test::MakeModbusBuffer( tcpFrame, modbus::FrameType::TCP ) );
     auto receiveBuffer = std::make_shared< modbus::ModbusBuffer >( modbus::FrameType::TCP );

     ExchangeMessage( []( const modbus_gateway::ModbusMessagePtr& in )->modbus_gateway::ModbusMessagePtr
                      {
                           modbus_gateway::ModbusMessageInfo modbusMessageInfo = in->GetModbusMessageInfo();
                           modbus_gateway::ModbusBufferPtr modbusBuffer = in->GetModbusBuffer();
                           modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper( *modbusBuffer );
                           auto id = modbusBufferTcpWrapper.GetTransactionId() + 1;
                           modbusBufferTcpWrapper.SetTransactionId( id );
                           return modbus_gateway::ModbusMessage::Create( modbusMessageInfo, modbusBuffer );
                      }, sendBuffer, receiveBuffer );

     EXPECT_TRUE( test::Compare( *sendBuffer, *receiveBuffer) );
}

TEST( ModbusTcpConnection, FailInputCheck )
{
     static const modbus::AduBuffer tcpFrame = { 0x0, 0x1, 0x0, 0x0, 0x0, 0x5, 0x1, 0x3, 0x4 };
     auto sendBuffer = std::make_shared< modbus::ModbusBuffer >(
     test::MakeModbusBuffer( tcpFrame, modbus::FrameType::TCP ) );
     auto receiveBuffer = std::make_shared< modbus::ModbusBuffer >( modbus::FrameType::TCP );

     bool haveMessage = false;

     ExchangeMessage( [ &haveMessage ]( const modbus_gateway::ModbusMessagePtr& in )->modbus_gateway::ModbusMessagePtr
                      {
                           haveMessage = true;
                           return nullptr;
                      }, sendBuffer, receiveBuffer );

     EXPECT_FALSE( haveMessage );
}

TEST( ModbusTcpConnection, NullptrMessage )
{
     static const modbus::AduBuffer tcpFrame = { 0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4 };
     auto sendBuffer = std::make_shared< modbus::ModbusBuffer >(
     test::MakeModbusBuffer( tcpFrame, modbus::FrameType::TCP ) );
     auto receiveBuffer = std::make_shared< modbus::ModbusBuffer >( modbus::FrameType::TCP );
     const size_t originSize = receiveBuffer->GetAduSize();

     ExchangeMessage( []( const modbus_gateway::ModbusMessagePtr& )->modbus_gateway::ModbusMessagePtr
                      {
                           return nullptr;
                      }, sendBuffer, receiveBuffer );

     EXPECT_EQ( originSize, receiveBuffer->GetAduSize() );
}

TEST( ModbusTcpConnection, InvalidMessageId )
{
     static const modbus::AduBuffer tcpFrame = { 0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4 };
     auto sendBuffer = std::make_shared< modbus::ModbusBuffer >(
     test::MakeModbusBuffer( tcpFrame, modbus::FrameType::TCP ) );
     auto receiveBuffer = std::make_shared< modbus::ModbusBuffer >( modbus::FrameType::TCP );
     const size_t originSize = receiveBuffer->GetAduSize();

     ExchangeMessage( []( const modbus_gateway::ModbusMessagePtr& in )->modbus_gateway::ModbusMessagePtr
                      {
                           modbus_gateway::ModbusMessageInfo info( in->GetModbusMessageInfo().GetTransactionId() + 1,
                                                                   in->GetModbusMessageInfo().GetSourceId() );
                           return modbus_gateway::ModbusMessage::Create( info, in->GetModbusBuffer() );
                      }, sendBuffer, receiveBuffer );

     EXPECT_EQ( originSize, receiveBuffer->GetAduSize() );
}

TEST( ModbusTcpConnection, InvalidSourceId )
{
     static const modbus::AduBuffer tcpFrame = { 0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4 };
     auto sendBuffer = std::make_shared< modbus::ModbusBuffer >(
     test::MakeModbusBuffer( tcpFrame, modbus::FrameType::TCP ) );
     auto receiveBuffer = std::make_shared< modbus::ModbusBuffer >( modbus::FrameType::TCP );
     const size_t originSize = receiveBuffer->GetAduSize();

     ExchangeMessage( []( const modbus_gateway::ModbusMessagePtr& in )->modbus_gateway::ModbusMessagePtr
                      {
                           modbus_gateway::ModbusMessageInfo info( in->GetModbusMessageInfo().GetTransactionId(),
                                                                   in->GetModbusMessageInfo().GetSourceId() + 1 );
                           return modbus_gateway::ModbusMessage::Create( info, in->GetModbusBuffer() );
                      }, sendBuffer, receiveBuffer );

     EXPECT_EQ( originSize, receiveBuffer->GetAduSize() );
}

TEST( ModbusTcpConnection, NullModbusBuffer )
{
     static const modbus::AduBuffer tcpFrame = { 0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4 };
     auto sendBuffer = std::make_shared< modbus::ModbusBuffer >(
     test::MakeModbusBuffer( tcpFrame, modbus::FrameType::TCP ) );
     auto receiveBuffer = std::make_shared< modbus::ModbusBuffer >( modbus::FrameType::TCP );
     const size_t originSize = receiveBuffer->GetAduSize();

     ExchangeMessage( []( const modbus_gateway::ModbusMessagePtr& in )->modbus_gateway::ModbusMessagePtr
                      {
                           return modbus_gateway::ModbusMessage::Create( in->GetModbusMessageInfo(), nullptr );
                      }, sendBuffer, receiveBuffer );

     EXPECT_EQ( originSize, receiveBuffer->GetAduSize() );
}
