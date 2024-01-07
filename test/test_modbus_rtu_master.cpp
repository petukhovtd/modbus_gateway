#include <gtest/gtest.h>

#include <common/context_runner.h>
#include <common/misc.h>
#include <common/rtu_creator.h>
#include <common/modbus_message_sender.h>
#include <common/test_modbus_rtu_slave.h>

#include <exchange/actor_storage_ht.h>
#include <exchange/exchange.h>

#include <modbus/modbus_buffer_rtu_wrapper.h>

#include <transport/modbus_rtu_master.h>

#include <thread>

struct ModbusRtuMasterTest : testing::Test {
protected:
  void SetUp() override {
    contextRunner.Run();
    auto context = contextRunner.GetContext();

    exchange = std::make_shared<exchange::Exchange>(std::make_unique<exchange::ActorStorageHT>());

    modbusMessageSender = test::ModbusMessageSender::Create(exchange);
    const exchange::ActorId modbusMessageSenderId = exchange->Add(modbusMessageSender);

    modbusRtuMaster = modbus_gateway::ModbusRtuMaster::Create(exchange, context, deviceIn, modbus_gateway::RtuOptions{}, messageTimeout, modbus::RTU);
    modbusRtuMasterId = exchange->Add(modbusRtuMaster);

    testModbusRtuSlave = std::make_unique<test::TestModbusRtuSlave>(context, deviceOut, modbus::RTU);
    testModbusRtuSlave->Start();
  }

  void TearDown() override {
    testModbusRtuSlave->Stop();
    contextRunner.Stop();
  }

  modbus_gateway::ModbusMessagePtr Process(const test::TestModbusRtuSlave::Handler &handler,
                                           const modbus_gateway::ModbusBufferPtr &sendBuffer) {
    testModbusRtuSlave->SetHandler(handler);
    modbusMessageSender->SendTo(sendBuffer, modbusRtuMasterId);

    std::this_thread::sleep_for(waitExchange);

    const auto answer = modbusMessageSender->receiveMessage;
    modbusMessageSender->receiveMessage = nullptr;

    return answer;
  }

  const std::string deviceIn = test::deviceIn;
  const std::string deviceOut = test::deviceOut;
  static constexpr auto waitExchange = std::chrono::milliseconds(2000);
  static constexpr auto messageTimeout = std::chrono::milliseconds(1000);

  test::ContextRunner contextRunner = test::ContextRunner{1};
  exchange::ExchangePtr exchange = nullptr;
  test::ModbusMessageSender::Ptr modbusMessageSender = nullptr;
  modbus_gateway::ModbusRtuMaster::Ptr modbusRtuMaster = nullptr;
  exchange::ActorId modbusRtuMasterId = 0;
  std::unique_ptr<test::TestModbusRtuSlave> testModbusRtuSlave = nullptr;
};

TEST_F(ModbusRtuMasterTest, EchoTest) {
  {
    static const modbus::AduBuffer tcpFrame = {0x1, 0x6, 0xDF, 0x62, 0x38};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::RTU));
    const auto answer = Process([](modbus_gateway::ModbusBufferPtr &buffer) {
      return buffer;
    },
                                sendBuffer);
    ASSERT_TRUE(answer);
    ASSERT_TRUE(answer->GetModbusBuffer());
    EXPECT_TRUE(test::Compare(answer->GetModbusBuffer().operator*(), *sendBuffer));
  }
}

TEST_F(ModbusRtuMasterTest, TimeoutTest) {
  {// timeout
    static const modbus::AduBuffer tcpFrame = {0x1, 0x6, 0xDF, 0x62, 0x38};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::RTU));
    const auto answer = Process([](modbus_gateway::ModbusBufferPtr &buffer) {
      return nullptr;
    },
                                sendBuffer);
    ASSERT_FALSE(answer);
  }
  {// good
    static const modbus::AduBuffer tcpFrame = {0x1, 0x6, 0xDF, 0x62, 0x38};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::RTU));
    const auto answer = Process([](modbus_gateway::ModbusBufferPtr &buffer) {
      return buffer;
    },
                                sendBuffer);
    ASSERT_TRUE(answer);
    ASSERT_TRUE(answer->GetModbusBuffer());
    EXPECT_TRUE(test::Compare(answer->GetModbusBuffer().operator*(), *sendBuffer));
  }
}

TEST_F(ModbusRtuMasterTest, CheckResponse) {
  {// invalid crc
    static const modbus::AduBuffer tcpFrame = {0x1, 0x6, 0xDF, 0x62, 0x38};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::RTU));
    const auto answer = Process([](modbus_gateway::ModbusBufferPtr &buffer) {
      ++ *buffer->begin().base();
      return buffer;
    },
                                sendBuffer);
    ASSERT_FALSE(answer);
  }
  {// good
    static const modbus::AduBuffer tcpFrame = {0x1, 0x6, 0xDF, 0x62, 0x38};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::RTU));
    const auto answer = Process([](modbus_gateway::ModbusBufferPtr &buffer) {
      return buffer;
    },
                                sendBuffer);
    ASSERT_TRUE(answer);
    ASSERT_TRUE(answer->GetModbusBuffer());
    EXPECT_TRUE(test::Compare(answer->GetModbusBuffer().operator*(), *sendBuffer));
  }
}
