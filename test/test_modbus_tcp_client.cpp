#include <gtest/gtest.h>

#include <common/context_runner.h>
#include <common/misc.h>
#include <common/modbus_message_sender.h>
#include <common/test_modbus_tcp_server.h>

#include <modbus/modbus_buffer_tcp_wrapper.h>

#include <transport/modbus_tcp_client.h>

#include <thread>

struct ModbusTcpClientTest : testing::Test {
protected:
  void SetUp() override {
    contextRunner.Run();
    auto context = contextRunner.GetContext();

    exchange = test::MakeExchange();

    modbusMessageSender = test::ModbusMessageSender::Create(exchange);
    const exchange::ActorId modbusMessageSenderId = exchange->Add(modbusMessageSender);

    modbusRtuMaster = modbus_gateway::ModbusTcpClient::Create(exchange, context, addr, port, messageTimeout);
    modbusRtuMasterId = exchange->Add(modbusRtuMaster);

    testModbusRtuSlave = std::make_unique<test::TestModbusTcpServer>(context, addr, port);
    testModbusRtuSlave->Start();
  }

  void TearDown() override {
    testModbusRtuSlave->Stop();
    contextRunner.Stop();
  }

  modbus_gateway::ModbusMessagePtr Process(const test::TestModbusTcpServer::Handler &handler,
                                           const modbus_gateway::ModbusBufferPtr &sendBuffer) {
    testModbusRtuSlave->SetHandler(handler);
    modbusMessageSender->SendTo(sendBuffer, modbusRtuMasterId);

    std::this_thread::sleep_for(waitExchange);

    const auto answer = modbusMessageSender->receiveMessage;
    modbusMessageSender->receiveMessage = nullptr;

    return answer;
  }

  const asio::ip::address_v4 addr = asio::ip::address_v4::loopback();
  const asio::ip::port_type port = 1234;
  static constexpr auto waitExchange = std::chrono::milliseconds(2000);
  static constexpr auto messageTimeout = std::chrono::milliseconds(1000);

  test::ContextRunner contextRunner = test::ContextRunner{1};
  exchange::ExchangePtr exchange = nullptr;
  test::ModbusMessageSender::Ptr modbusMessageSender = nullptr;
  modbus_gateway::ModbusTcpClient::Ptr modbusRtuMaster = nullptr;
  exchange::ActorId modbusRtuMasterId = 0;
  std::unique_ptr<test::TestModbusTcpServer> testModbusRtuSlave = nullptr;
};

TEST_F(ModbusTcpClientTest, ConnectAndSendTest) {
  {
    static const modbus::AduBuffer tcpFrame = {0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::TCP));
    const auto answer = Process([](modbus_gateway::ModbusBufferPtr &buffer) {
      return buffer;
    },
                                sendBuffer);
    ASSERT_TRUE(answer);
    ASSERT_TRUE(answer->GetModbusBuffer());
    EXPECT_TRUE(test::Compare(answer->GetModbusBuffer().operator*(), *sendBuffer));
  }
  {
    static const modbus::AduBuffer tcpFrame = {0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::TCP));
    const auto answer = Process([](modbus_gateway::ModbusBufferPtr &buffer) {
      return buffer;
    },
                                sendBuffer);
    ASSERT_TRUE(answer);
    ASSERT_TRUE(answer->GetModbusBuffer());
    EXPECT_TRUE(test::Compare(answer->GetModbusBuffer().operator*(), *sendBuffer));
  }
}

TEST_F(ModbusTcpClientTest, TimeoutTest) {
  {// timeout
    static const modbus::AduBuffer tcpFrame = {0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::TCP));
    const auto answer = Process([](modbus_gateway::ModbusBufferPtr &buffer) {
      return nullptr;
    },
                                sendBuffer);
    ASSERT_FALSE(answer);
  }
  {// good
    static const modbus::AduBuffer tcpFrame = {0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::TCP));
    const auto answer = Process([](modbus_gateway::ModbusBufferPtr &buffer) {
      return buffer;
    },
                                sendBuffer);
    ASSERT_TRUE(answer);
    ASSERT_TRUE(answer->GetModbusBuffer());
    EXPECT_TRUE(test::Compare(answer->GetModbusBuffer().operator*(), *sendBuffer));
  }
}

TEST_F(ModbusTcpClientTest, CheckResponse) {
  {// invalid tr id
    static const modbus::AduBuffer tcpFrame = {0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::TCP));
    const auto answer = Process([](modbus_gateway::ModbusBufferPtr &buffer) {
      modbus::ModbusBufferTcpWrapper wrapper(*buffer);
      auto id = wrapper.GetTransactionId();
      wrapper.SetTransactionId(++id);
      return buffer;
    },
                                sendBuffer);
    ASSERT_FALSE(answer);
  }
  {// invalid frame
    static const modbus::AduBuffer tcpFrame = {0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::TCP));
    const auto answer = Process([](modbus_gateway::ModbusBufferPtr &) {
      const modbus::AduBuffer tcpFrame = {0x0, 0x1, 0x0, 0x1, 0x0, 0x3, 0x1, 0x3, 0x4};
      auto buffer = std::make_shared<modbus::ModbusBuffer>(
          test::MakeModbusBuffer(tcpFrame, modbus::FrameType::TCP));
      return buffer;
    },
                                sendBuffer);
    ASSERT_FALSE(answer);
  }
  {// good
    static const modbus::AduBuffer tcpFrame = {0x0, 0x1, 0x0, 0x0, 0x0, 0x3, 0x1, 0x3, 0x4};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::TCP));
    const auto answer = Process([](modbus_gateway::ModbusBufferPtr &buffer) {
      return buffer;
    },
                                sendBuffer);
    ASSERT_TRUE(answer);
    ASSERT_TRUE(answer->GetModbusBuffer());
    EXPECT_TRUE(test::Compare(answer->GetModbusBuffer().operator*(), *sendBuffer));
  }
}
