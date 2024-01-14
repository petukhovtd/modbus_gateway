#include <gtest/gtest.h>

#include <common/context_runner.h>
#include <common/misc.h>

#include <exchange/actor_storage_ht.h>

#include <common/modbus_message_actor.h>
#include <common/single_router.h>

#include <common/test_modbus_tcp_client.h>

#include <exchange/exchange.h>
#include <message/modbus_message.h>
#include <modbus/modbus_buffer_tcp_wrapper.h>
#include <thread>
#include <transport/modbus_tcp_server.h>


struct ModbusTcpConnectionTest : testing::Test {

protected:
  void SetUp() override {
    contextRunner.Run();
    auto context = contextRunner.GetContext();

    exchange = std::make_shared<exchange::Exchange>(std::make_unique<exchange::ActorStorageHT>());
    modbusMessageSender = test::ModbusMessageActor::Create(exchange);

    const exchange::ActorId modbusEchoActorId = exchange->Add(modbusMessageSender);
    modbus_gateway::RouterPtr singleRouter = std::make_shared<test::SingleRouter>(modbusEchoActorId);
    tcpServer = modbus_gateway::ModbusTcpServer::Create(exchange, context, asio::ip::address(addr), port,
                                                        singleRouter);
    exchange->Add(tcpServer);
    tcpServer->Start();

    testConnection = std::make_unique<test::TestModbusTcpClient>(context, asio::ip::address(addr), port);

    ASSERT_FALSE(testConnection->Connect());
  }

  void TearDown() override {
    testConnection->Disconnect();
    tcpServer->Stop();
    contextRunner.Stop();
  }

  void Process(const test::ModbusMessageActor::Handler &handler,
               const modbus_gateway::ModbusBufferPtr &sendBuffer,
               const modbus_gateway::ModbusBufferPtr &receiveBuffer) {
    modbusMessageSender->SetHandler(handler);
    testConnection->Process(sendBuffer, receiveBuffer);

    std::this_thread::sleep_for(waitExchange);
  }

  const asio::ip::address_v4 addr = asio::ip::address_v4::loopback();
  const asio::ip::port_type port = 1234;
  static constexpr auto waitExchange = std::chrono::milliseconds(100);

  test::ContextRunner contextRunner = test::ContextRunner{1};
  exchange::ExchangePtr exchange = nullptr;
  test::ModbusMessageActor::Ptr modbusMessageSender = nullptr;
  modbus_gateway::ModbusTcpServer::Ptr tcpServer = nullptr;
  std::unique_ptr<test::TestModbusTcpClient> testConnection = nullptr;
};

TEST_F(ModbusTcpConnectionTest, EchoTest) {
  static const modbus::AduBuffer tcpFrame = {0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4};
  auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
      test::MakeModbusBuffer(tcpFrame, modbus::FrameType::TCP));
  auto receiveBuffer = std::make_shared<modbus::ModbusBuffer>(modbus::FrameType::TCP);

  Process([](const modbus_gateway::ModbusMessagePtr &in) -> modbus_gateway::ModbusMessagePtr {
    return in;
  },
          sendBuffer, receiveBuffer);

  EXPECT_TRUE(test::Compare(*sendBuffer, *receiveBuffer));
}

TEST_F(ModbusTcpConnectionTest, SaveTransactionId) {
  static const modbus::AduBuffer tcpFrame = {0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4};
  auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
      test::MakeModbusBuffer(tcpFrame, modbus::FrameType::TCP));
  auto receiveBuffer = std::make_shared<modbus::ModbusBuffer>(modbus::FrameType::TCP);

  Process([](const modbus_gateway::ModbusMessagePtr &in) -> modbus_gateway::ModbusMessagePtr {
    modbus_gateway::ModbusMessageInfo modbusMessageInfo = in->GetModbusMessageInfo();
    modbus_gateway::ModbusBufferPtr modbusBuffer = in->GetModbusBuffer();
    modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper(*modbusBuffer);
    auto id = modbusBufferTcpWrapper.GetTransactionId() + 1;
    modbusBufferTcpWrapper.SetTransactionId(id);
    return modbus_gateway::ModbusMessage::Create(modbusMessageInfo, modbusBuffer);
  },
          sendBuffer, receiveBuffer);

  EXPECT_TRUE(test::Compare(*sendBuffer, *receiveBuffer));
}

TEST_F(ModbusTcpConnectionTest, CheckRequest) {
  auto receiveBuffer = std::make_shared<modbus::ModbusBuffer>(modbus::FrameType::TCP);
  size_t messageCount = 0;

  auto handler = [&messageCount](const modbus_gateway::ModbusMessagePtr &in) -> modbus_gateway::ModbusMessagePtr {
    ++messageCount;
    return in;
  };

  {// invalid size for tcp
    static const modbus::AduBuffer tcpFrame = {0x3, 0x1, 0x3, 0x4};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::RTU));
    Process(handler, sendBuffer, receiveBuffer);
    EXPECT_EQ(messageCount, 0);
  }

  {// invalid type for tcp
    static const modbus::AduBuffer tcpFrame = {0x10, 0x0, 0x0, 0x0, 0x1, 0x2, 0x0, 0x0};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::RTU));
    Process(handler, sendBuffer, receiveBuffer);
    EXPECT_EQ(messageCount, 0);
  }
}

TEST_F(ModbusTcpConnectionTest, CheckResponse) {
  static const modbus::AduBuffer tcpFrame = {0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4};
  auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
      test::MakeModbusBuffer(tcpFrame, modbus::FrameType::TCP));
  auto receiveBuffer = std::make_shared<modbus::ModbusBuffer>(modbus::FrameType::TCP);
  auto defaultSize = receiveBuffer->GetAduSize();

  {// invalid source id
    Process([](const modbus_gateway::ModbusMessagePtr &in) -> modbus_gateway::ModbusMessagePtr {
      auto mi = in->GetModbusMessageInfo();
      modbus_gateway::ModbusMessageInfo messageInfo(mi.GetSourceId() + 1, mi.GetTransactionId());
      auto response = modbus_gateway::ModbusMessage::Create(messageInfo, in->GetModbusBuffer());
      return response;
    },
            sendBuffer, receiveBuffer);

    EXPECT_EQ(defaultSize, receiveBuffer->GetAduSize());
  }
  {// invalid transaction id
    Process([](const modbus_gateway::ModbusMessagePtr &in) -> modbus_gateway::ModbusMessagePtr {
      auto mi = in->GetModbusMessageInfo();
      modbus_gateway::ModbusMessageInfo messageInfo(mi.GetSourceId(), mi.GetTransactionId() + 1);
      auto response = modbus_gateway::ModbusMessage::Create(messageInfo, in->GetModbusBuffer());
      return response;
    },
            sendBuffer, receiveBuffer);

    EXPECT_EQ(defaultSize, receiveBuffer->GetAduSize());
  }
  {// null buffer
    Process([](const modbus_gateway::ModbusMessagePtr &in) -> modbus_gateway::ModbusMessagePtr {
      auto response = modbus_gateway::ModbusMessage::Create(in->GetModbusMessageInfo(), nullptr);
      return response;
    },
            sendBuffer, receiveBuffer);

    EXPECT_EQ(defaultSize, receiveBuffer->GetAduSize());
  }
}
