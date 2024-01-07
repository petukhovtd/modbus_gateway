#include <gtest/gtest.h>

#include <common/context_runner.h>
#include <common/misc.h>
#include <common/modbus_message_actor.h>
#include <common/single_router.h>
#include <common/test_modbus_rtu_master.h>
#include <common/rtu_creator.h>

#include <exchange/actor_storage_ht.h>
#include <exchange/exchange.h>

#include <transport/modbus_rtu_slave.h>

#include <thread>

struct ModbusRtuSlaveTest : testing::Test {

protected:
  void SetUp() override {
    contextRunner.Run();
    auto context = contextRunner.GetContext();

    exchange = std::make_shared<exchange::Exchange>(std::make_unique<exchange::ActorStorageHT>());
    modbusMessageSender = test::ModbusMessageActor::Create(exchange);

    const exchange::ActorId modbusEchoActorId = exchange->Add(modbusMessageSender);
    modbus_gateway::RouterPtr singleRouter = std::make_shared<test::SingleRouter>(modbusEchoActorId);
    modbusRtuSlave = modbus_gateway::ModbusRtuSlave::Create(exchange, context, deviceOut, modbus_gateway::RtuOptions{}, singleRouter, modbus::RTU);
    exchange->Add(modbusRtuSlave);

    modbusRtuSlave->Start();

    modbusRtuMaster = std::make_unique<test::TestModbusRtuMaster>(context, deviceIn);
  }

  void TearDown() override {
    modbusRtuMaster->Cancel();
    contextRunner.Stop();
  }

  void Process(const test::ModbusMessageActor::Handler &handler,
               const modbus_gateway::ModbusBufferPtr &sendBuffer,
               const modbus_gateway::ModbusBufferPtr &receiveBuffer) {
    modbusMessageSender->SetHandler(handler);
    modbusRtuMaster->Process(sendBuffer, receiveBuffer);

    std::this_thread::sleep_for(waitExchange);
  }

  static constexpr auto waitExchange = std::chrono::milliseconds(100);
  const std::string deviceIn = test::deviceIn;
  const std::string deviceOut = test::deviceOut;

  test::ContextRunner contextRunner = test::ContextRunner{1};
  exchange::ExchangePtr exchange = nullptr;
  test::ModbusMessageActor::Ptr modbusMessageSender = nullptr;
  modbus_gateway::ModbusRtuSlave::Ptr modbusRtuSlave = nullptr;
  std::unique_ptr<test::TestModbusRtuMaster> modbusRtuMaster = nullptr;
};

TEST_F(ModbusRtuSlaveTest, EchoTest) {
  static const modbus::AduBuffer tcpFrame = {0x1, 0x6, 0xDF, 0x62, 0x38};
  auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
      test::MakeModbusBuffer(tcpFrame, modbus::FrameType::RTU));
  auto receiveBuffer = std::make_shared<modbus::ModbusBuffer>(modbus::FrameType::RTU);

  Process([](const modbus_gateway::ModbusMessagePtr &in) -> modbus_gateway::ModbusMessagePtr {
    return in;
  },
          sendBuffer, receiveBuffer);

  EXPECT_TRUE(test::Compare(*sendBuffer, *receiveBuffer));
}

TEST_F(ModbusRtuSlaveTest, CheckRequest) {
  auto receiveBuffer = std::make_shared<modbus::ModbusBuffer>(modbus::FrameType::RTU);
  size_t messageCount = 0;

  auto handler = [&messageCount](const modbus_gateway::ModbusMessagePtr &in) -> modbus_gateway::ModbusMessagePtr {
    ++messageCount;
    return in;
  };

  {// invalid crc
    static const modbus::AduBuffer tcpFrame = {0x1, 0x6, 0xDF, 0x62, 0x40};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::RTU));
    Process(handler, sendBuffer, receiveBuffer);
    EXPECT_EQ(messageCount, 0);
  }
  {// invalid crc
    static const modbus::AduBuffer tcpFrame = {0x2, 0x6, 0xDF, 0x62, 0x38};
    auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
        test::MakeModbusBuffer(tcpFrame, modbus::FrameType::RTU));
    Process(handler, sendBuffer, receiveBuffer);
    EXPECT_EQ(messageCount, 0);
  }
}

TEST_F(ModbusRtuSlaveTest, CheckResponse) {
  static const modbus::AduBuffer tcpFrame = {0x1, 0x6, 0xDF, 0x62, 0x38};
  auto sendBuffer = std::make_shared<modbus::ModbusBuffer>(
      test::MakeModbusBuffer(tcpFrame, modbus::FrameType::RTU));
  auto receiveBuffer = std::make_shared<modbus::ModbusBuffer>(modbus::FrameType::RTU);
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
