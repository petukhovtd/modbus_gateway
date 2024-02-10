#include <gtest/gtest.h>

#include <config/config.h>
#include <config/config_service.h>
#include <config/extractor.h>
#include <config/rtu_maser_config.h>
#include <config/rtu_slave_config.h>
#include <config/tcp_client_config.h>
#include <config/tcp_server_config.h>
#include <config/trace_deep.h>
#include <config/trace_path.h>

#include <sstream>

TEST(ConfigTest, ParsingExampleTest) {
  std::stringstream is;
  is << R"(
{
  "service": {
    "log_level": "error",
    "threads": 5
  },
  "slaves": [
    {
      "frame_type": "tcp",
      "ip_address": "192.168.1.2",
      "ip_port": 503
    },
    {
      "frame_type": "tcp",
      "ip_port": 502
    },
    {
      "frame_type": "rtu",
      "device": "/dev/ttyUSB0",
      "baud_rate": 115200,
      "character_size": 8,
      "parity": "even",
      "stop_bits": 1,
      "flow_control": "none"
    },
    {
      "frame_type": "ascii",
      "device": "/dev/ttyUSB1",
      "baud_rate": 9600,
      "character_size": 8,
      "parity": "none",
      "stop_bits": 2,
      "flow_control": "none"
    }
  ],
  "masters": [
    {
      "frame_type": "tcp",
      "timeout_ms": 1000,
      "ip_address": "192.168.2.2",
      "ip_port": 502
    },
    {
      "frame_type": "rtu",
      "timeout_ms": 1000,
      "device": "/dev/ttyUSB2",
      "baud_rate": 115200,
      "character_size": 8,
      "parity": "even",
      "stop_bits": 1,
      "flow_control": "none",
      "unit_id": [
        {
          "type": "range",
          "begin": 10,
          "end": 20
        },
        {
          "type": "value",
          "value": 25
        }
      ]
    },
    {
      "frame_type": "ascii",
      "timeout_ms": 1000,
      "device": "/dev/ttyUSB3",
      "baud_rate": 9600,
      "character_size": 8,
      "parity": "none",
      "stop_bits": 2,
      "flow_control": "none",
      "rs485": {
        "rts_on_send": true,
        "rts_after_send": false,
        "rx_during_tx": true,
        "terminate_bus": false,
        "delay_rts_before_send": 1000,
        "delay_rts_after_send": 2000
      },
      "unit_id": [
        {
          "type": "value",
          "value": 30
        },
        {
          "type": "range",
          "begin": 40,
          "end": 50
        },
        {
          "type": "value",
          "value": 55
        }
      ]
    },
    {
      "frame_type": "tcp",
      "timeout_ms": 1000,
      "ip_address": "192.168.3.2",
      "ip_port": 503,
      "unit_id": [
        {
          "type": "range",
          "begin": 100,
          "end": 110
        }
      ]
    }
  ]
}
)";
  std::optional<modbus_gateway::Config> config;
  ASSERT_NO_THROW(config = modbus_gateway::Config(is));
  ASSERT_TRUE(config);
  EXPECT_EQ(config->configService.logLevel, modbus_gateway::Logger::LogLevel::Error);
  EXPECT_EQ(config->configService.threads, 5);
  EXPECT_EQ(config->slaves.size(), 4);
  EXPECT_EQ(config->masters.size(), 4);
  EXPECT_NO_THROW(config->Validate());
}

TEST(ConfigTest, ManyDefaultsTest) {
  std::stringstream is;
  is << R"(
{
  "service": {
  },
  "slaves": [
    {
      "frame_type": "tcp",
      "ip_address": "192.168.1.2",
      "ip_port": 503
    }
  ],
  "masters": [
    {
      "frame_type": "tcp",
      "timeout_ms": 1000,
      "ip_address": "192.168.2.2",
      "ip_port": 502
    },
    {
      "frame_type": "rtu",
      "timeout_ms": 1000,
      "device": "/dev/ttyUSB0"
    }
  ]
}
)";
  std::optional<modbus_gateway::Config> config;
  ASSERT_NO_THROW(config = modbus_gateway::Config(is));
  ASSERT_TRUE(config);
  EXPECT_EQ(config->slaves.size(), 1);
  EXPECT_EQ(config->masters.size(), 2);
  EXPECT_THROW(config->Validate(), modbus_gateway::InvalidValueException);
}

TEST(ConfigTest, ServiceSectionTest) {
  std::stringstream is;
  is << R"(
{
  "service": {
    "log_level": "error",
    "threads": 5
  }
}
)";
  auto data = nlohmann::json::parse(is);
  modbus_gateway::TracePath tracePath;
  tracePath.Push("config");

  modbus_gateway::ConfigService configService;
  EXPECT_NO_THROW(configService = modbus_gateway::ExtractConfigService(tracePath, data));
  EXPECT_EQ(configService.logLevel, modbus_gateway::Logger::LogLevel::Error);
  EXPECT_EQ(configService.threads, 5);
}

TEST(ConfigTest, ServiceSectionOptionalTest) {
  std::stringstream is;
  is << R"(
{
  "service": {
  }
}
)";
  auto data = nlohmann::json::parse(is);
  modbus_gateway::TracePath tracePath;
  tracePath.Push("config");

  modbus_gateway::ConfigService configService;
  EXPECT_NO_THROW(configService = modbus_gateway::ExtractConfigService(tracePath, data));
  EXPECT_EQ(configService.logLevel, modbus_gateway::Logger::LogLevel::Info);
  EXPECT_EQ(configService.threads, 1);
}

TEST(ConfigTest, SlaveTcpTest) {
  std::stringstream is;
  is << R"(
{
  "slave": {
    "frame_type": "tcp",
    "ip_address": "192.168.1.2",
    "ip_port": 1234
  }
}
)";
  auto data = nlohmann::json::parse(is);
  modbus_gateway::TracePath tp;
  modbus_gateway::TraceDeep td(tp, "slave");

  auto slave = modbus_gateway::FindObject(td, data);

  auto slavePtr = modbus_gateway::ExtractSlave(td.GetTracePath(), slave);

  ASSERT_TRUE(slavePtr);
  ASSERT_EQ(slavePtr->GetType(), modbus_gateway::TransportType::TcpServer);

  auto tcpServerConfig = std::dynamic_pointer_cast<modbus_gateway::TcpServerConfig>(slavePtr);
  ASSERT_TRUE(tcpServerConfig);

  EXPECT_EQ(tcpServerConfig->address, asio::ip::address::from_string("192.168.1.2"));
  EXPECT_EQ(tcpServerConfig->port, 1234);
}

TEST(ConfigTest, SlaveTcpOptionalTest) {
  std::stringstream is;
  is << R"(
{
  "slave": {
    "frame_type": "tcp",
    "ip_port": 4321
  }
}
)";
  auto data = nlohmann::json::parse(is);
  modbus_gateway::TracePath tp;
  modbus_gateway::TraceDeep td(tp, "slave");

  auto slave = modbus_gateway::FindObject(td, data);

  auto slavePtr = modbus_gateway::ExtractSlave(td.GetTracePath(), slave);

  ASSERT_TRUE(slavePtr);
  ASSERT_EQ(slavePtr->GetType(), modbus_gateway::TransportType::TcpServer);

  auto tcpServerConfig = std::dynamic_pointer_cast<modbus_gateway::TcpServerConfig>(slavePtr);
  ASSERT_TRUE(tcpServerConfig);

  EXPECT_EQ(tcpServerConfig->address, asio::ip::address_v4::any());
  EXPECT_EQ(tcpServerConfig->port, 4321);
}

TEST(ConfigTest, SlaveRtuTest) {
  std::stringstream is;
  is << R"(
{
  "slave": {
    "frame_type": "rtu",
    "device": "/dev/ttyUSB0",
    "baud_rate": 115200,
    "character_size": 7,
    "parity": "even",
    "stop_bits": 1.5,
    "flow_control": "software",
    "rs485": {
      "rts_on_send": true,
      "rts_after_send": false,
      "rx_during_tx": true,
      "terminate_bus": false,
      "delay_rts_before_send": 1000,
      "delay_rts_after_send": 2000
    }
  },
  "slave2": {
    "frame_type": "ascii",
    "device": "/dev/ttyUSB1",
    "baud_rate": 9600,
    "character_size": 8,
    "parity": "none",
    "stop_bits": 2,
    "flow_control": "none"
  }
}
)";
  auto data = nlohmann::json::parse(is);
  modbus_gateway::TracePath tp;
  {
    modbus_gateway::TraceDeep td(tp, "slave");

    auto slave = modbus_gateway::FindObject(td, data);

    auto slavePtr = modbus_gateway::ExtractSlave(td.GetTracePath(), slave);

    ASSERT_TRUE(slavePtr);
    ASSERT_EQ(slavePtr->GetType(), modbus_gateway::TransportType::RtuSlave);

    auto rtuSlaveConfig = std::dynamic_pointer_cast<modbus_gateway::RtuSlaveConfig>(slavePtr);
    ASSERT_TRUE(rtuSlaveConfig);

    EXPECT_EQ(rtuSlaveConfig->GetFrameType(), modbus::RTU);
    EXPECT_STREQ(rtuSlaveConfig->device.c_str(), "/dev/ttyUSB0");

    EXPECT_EQ(rtuSlaveConfig->rtuOptions.baudRate.value(), asio::serial_port_base::baud_rate(115200).value());
    EXPECT_EQ(rtuSlaveConfig->rtuOptions.characterSize.value(), asio::serial_port_base::character_size(7).value());
    EXPECT_EQ(rtuSlaveConfig->rtuOptions.parity.value(), asio::serial_port_base::parity(asio::serial_port_base::parity::even).value());
    EXPECT_EQ(rtuSlaveConfig->rtuOptions.stopBits.value(), asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::onepointfive).value());
    EXPECT_EQ(rtuSlaveConfig->rtuOptions.flowControl.value(), asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::software).value());

    ASSERT_TRUE(rtuSlaveConfig->rtuOptions.rs485.has_value());
    const auto& rs485 = rtuSlaveConfig->rtuOptions.rs485.value();
    ASSERT_TRUE(rs485.rtsOnSend.has_value());
    EXPECT_TRUE(rs485.rtsOnSend.value());
    ASSERT_TRUE(rs485.rtsAfterSend.has_value());
    EXPECT_FALSE(rs485.rtsAfterSend.value());
    ASSERT_TRUE(rs485.rxDuringTx.has_value());
    EXPECT_TRUE(rs485.rxDuringTx.value());
    ASSERT_TRUE(rs485.terminateBus.has_value());
    EXPECT_FALSE(rs485.terminateBus.value());
    ASSERT_TRUE(rs485.delayRtsBeforeSend.has_value());
    EXPECT_EQ(rs485.delayRtsBeforeSend.value(),1000);
    ASSERT_TRUE(rs485.delayRtsAfterSend.has_value());
    EXPECT_EQ(rs485.delayRtsAfterSend.value(),2000);
  }
  {
    modbus_gateway::TraceDeep td(tp, "slave2");

    auto slave = modbus_gateway::FindObject(td, data);

    auto slavePtr = modbus_gateway::ExtractSlave(td.GetTracePath(), slave);

    ASSERT_TRUE(slavePtr);
    ASSERT_EQ(slavePtr->GetType(), modbus_gateway::TransportType::RtuSlave);

    auto rtuSlaveConfig = std::dynamic_pointer_cast<modbus_gateway::RtuSlaveConfig>(slavePtr);
    ASSERT_TRUE(rtuSlaveConfig);

    EXPECT_EQ(rtuSlaveConfig->GetFrameType(), modbus::ASCII);
  }
}

TEST(ConfigTest, SlaveRtuOptionalTest) {
  std::stringstream is;
  is << R"(
{
  "slave": {
    "frame_type": "rtu",
    "device": "/dev/ttyUSB0"
  }
}
)";
  auto data = nlohmann::json::parse(is);
  modbus_gateway::TracePath tp;
  {
    modbus_gateway::TraceDeep td(tp, "slave");

    auto slave = modbus_gateway::FindObject(td, data);

    auto slavePtr = modbus_gateway::ExtractSlave(td.GetTracePath(), slave);

    ASSERT_TRUE(slavePtr);
    ASSERT_EQ(slavePtr->GetType(), modbus_gateway::TransportType::RtuSlave);

    auto rtuSlaveConfig = std::dynamic_pointer_cast<modbus_gateway::RtuSlaveConfig>(slavePtr);
    ASSERT_TRUE(rtuSlaveConfig);

    EXPECT_EQ(rtuSlaveConfig->GetFrameType(), modbus::RTU);
    EXPECT_STREQ(rtuSlaveConfig->device.c_str(), "/dev/ttyUSB0");
    EXPECT_EQ(rtuSlaveConfig->rtuOptions.baudRate.value(), asio::serial_port_base::baud_rate().value());
    EXPECT_EQ(rtuSlaveConfig->rtuOptions.characterSize.value(), asio::serial_port_base::character_size().value());
    EXPECT_EQ(rtuSlaveConfig->rtuOptions.parity.value(), asio::serial_port_base::parity().value());
    EXPECT_EQ(rtuSlaveConfig->rtuOptions.stopBits.value(), asio::serial_port_base::stop_bits().value());
    EXPECT_EQ(rtuSlaveConfig->rtuOptions.flowControl.value(), asio::serial_port_base::flow_control().value());
    EXPECT_FALSE(rtuSlaveConfig->rtuOptions.rs485.has_value());
  }
}

TEST(ConfigTest, MasterTcpTest) {
  std::stringstream is;
  is << R"(
{
  "master": {
    "frame_type": "tcp",
    "timeout_ms": 1234,
    "ip_address": "192.168.3.2",
    "ip_port": 444,
    "unit_id": [
      {
        "type": "range",
        "begin": 100,
        "end": 110
      }
    ]
  }
}
)";
  auto data = nlohmann::json::parse(is);
  modbus_gateway::TracePath tp;
  modbus_gateway::TraceDeep td(tp, "master");

  auto master = modbus_gateway::FindObject(td, data);

  auto masterPtr = modbus_gateway::ExtractMaster(td.GetTracePath(), master);

  ASSERT_TRUE(masterPtr);
  ASSERT_EQ(masterPtr->GetType(), modbus_gateway::TransportType::TcpClient);

  auto tcpMasterConfig = std::dynamic_pointer_cast<modbus_gateway::TcpClientConfig>(masterPtr);
  ASSERT_TRUE(tcpMasterConfig);

  EXPECT_EQ(tcpMasterConfig->address, asio::ip::address::from_string("192.168.3.2"));
  EXPECT_EQ(tcpMasterConfig->port, 444);
  EXPECT_EQ(tcpMasterConfig->timeout.count(), 1234);
  EXPECT_EQ(tcpMasterConfig->unitIdSet.size(), 1);
}

TEST(ConfigTest, MasterTcpOptionalTest) {
  std::stringstream is;
  is << R"(
{
  "master": {
    "frame_type": "tcp",
    "timeout_ms": 4444,
    "ip_address": "192.168.3.7",
    "ip_port": 555
  }
}
)";
  auto data = nlohmann::json::parse(is);
  modbus_gateway::TracePath tp;
  modbus_gateway::TraceDeep td(tp, "master");

  auto master = modbus_gateway::FindObject(td, data);

  auto masterPtr = modbus_gateway::ExtractMaster(td.GetTracePath(), master);

  ASSERT_TRUE(masterPtr);
  ASSERT_EQ(masterPtr->GetType(), modbus_gateway::TransportType::TcpClient);

  auto tcpMasterConfig = std::dynamic_pointer_cast<modbus_gateway::TcpClientConfig>(masterPtr);
  ASSERT_TRUE(tcpMasterConfig);

  EXPECT_EQ(tcpMasterConfig->address, asio::ip::address::from_string("192.168.3.7"));
  EXPECT_EQ(tcpMasterConfig->port, 555);
  EXPECT_EQ(tcpMasterConfig->timeout.count(), 4444);
  EXPECT_EQ(tcpMasterConfig->unitIdSet.size(), 0);
}

TEST(ConfigTest, MasterRtuTest) {
  std::stringstream is;
  is << R"(
{
  "master": {
    "frame_type": "rtu",
    "timeout_ms": 3214,
    "device": "/dev/ttyUSB2",
    "baud_rate": 8800,
    "character_size": 6,
    "parity": "odd",
    "stop_bits": 2,
    "flow_control": "hardware",
    "rs485": {
      "rts_on_send": false,
      "rts_after_send": true,
      "rx_during_tx": false,
      "terminate_bus": true,
      "delay_rts_before_send": 3,
      "delay_rts_after_send": 9999
    },
    "unit_id": [
      {
        "type": "range",
        "begin": 10,
        "end": 20
      },
      {
        "type": "value",
        "value": 25
      }
    ]
  },
  "master2": {
    "frame_type": "ascii",
    "timeout_ms": 5432,
    "device": "/dev/ttyUSB3",
    "baud_rate": 3200,
    "character_size": 7,
    "parity": "none",
    "stop_bits": 1,
    "flow_control": "none",
    "unit_id": [
      {
        "type": "value",
        "value": 30
      },
      {
        "type": "range",
        "begin": 40,
        "end": 50
      },
      {
        "type": "value",
        "value": 55
      }
    ]
  }
}
)";
  auto data = nlohmann::json::parse(is);
  modbus_gateway::TracePath tp;
  {
    modbus_gateway::TraceDeep td(tp, "master");

    auto master = modbus_gateway::FindObject(td, data);

    auto masterPtr = modbus_gateway::ExtractMaster(td.GetTracePath(), master);

    ASSERT_TRUE(masterPtr);
    ASSERT_EQ(masterPtr->GetType(), modbus_gateway::TransportType::RtuMaster);

    auto rtuMasterConfig = std::dynamic_pointer_cast<modbus_gateway::RtuMasterConfig>(masterPtr);
    ASSERT_TRUE(rtuMasterConfig);

    EXPECT_EQ(rtuMasterConfig->GetFrameType(), modbus::RTU);
    EXPECT_STREQ(rtuMasterConfig->device.c_str(), "/dev/ttyUSB2");
    EXPECT_EQ(rtuMasterConfig->timeout.count(), 3214);
    EXPECT_EQ(rtuMasterConfig->rtuOptions.baudRate.value(), asio::serial_port_base::baud_rate(8800).value());
    EXPECT_EQ(rtuMasterConfig->rtuOptions.characterSize.value(), asio::serial_port_base::character_size(6).value());
    EXPECT_EQ(rtuMasterConfig->rtuOptions.parity.value(), asio::serial_port_base::parity(asio::serial_port_base::parity::odd).value());
    EXPECT_EQ(rtuMasterConfig->rtuOptions.stopBits.value(), asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::two).value());
    EXPECT_EQ(rtuMasterConfig->rtuOptions.flowControl.value(), asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::hardware).value());
    EXPECT_EQ(rtuMasterConfig->unitIdSet.size(), 2);

    ASSERT_TRUE(rtuMasterConfig->rtuOptions.rs485.has_value());
    const auto& rs485 = rtuMasterConfig->rtuOptions.rs485.value();
    ASSERT_TRUE(rs485.rtsOnSend.has_value());
    EXPECT_FALSE(rs485.rtsOnSend.value());
    ASSERT_TRUE(rs485.rtsAfterSend.has_value());
    EXPECT_TRUE(rs485.rtsAfterSend.value());
    ASSERT_TRUE(rs485.rxDuringTx.has_value());
    EXPECT_FALSE(rs485.rxDuringTx.value());
    ASSERT_TRUE(rs485.terminateBus.has_value());
    EXPECT_TRUE(rs485.terminateBus.value());
    ASSERT_TRUE(rs485.delayRtsBeforeSend.has_value());
    EXPECT_EQ(rs485.delayRtsBeforeSend.value(),3);
    ASSERT_TRUE(rs485.delayRtsAfterSend.has_value());
    EXPECT_EQ(rs485.delayRtsAfterSend.value(),9999);
  }
  {
    modbus_gateway::TraceDeep td(tp, "master2");

    auto master = modbus_gateway::FindObject(td, data);

    auto masterPtr = modbus_gateway::ExtractMaster(td.GetTracePath(), master);

    ASSERT_TRUE(masterPtr);
    ASSERT_EQ(masterPtr->GetType(), modbus_gateway::TransportType::RtuMaster);

    auto rtuMasterConfig = std::dynamic_pointer_cast<modbus_gateway::RtuMasterConfig>(masterPtr);
    ASSERT_TRUE(rtuMasterConfig);

    EXPECT_EQ(rtuMasterConfig->GetFrameType(), modbus::ASCII);
  }
}

TEST(ConfigTest, MasterRtuOptinalTest) {
  std::stringstream is;
  is << R"(
{
  "master": {
    "frame_type": "rtu",
    "timeout_ms": 3214,
    "device": "/dev/ttyUSB2"
  }
}
)";
  auto data = nlohmann::json::parse(is);
  modbus_gateway::TracePath tp;
  {
    modbus_gateway::TraceDeep td(tp, "master");

    auto master = modbus_gateway::FindObject(td, data);

    auto masterPtr = modbus_gateway::ExtractMaster(td.GetTracePath(), master);

    ASSERT_TRUE(masterPtr);
    ASSERT_EQ(masterPtr->GetType(), modbus_gateway::TransportType::RtuMaster);

    auto rtuMasterConfig = std::dynamic_pointer_cast<modbus_gateway::RtuMasterConfig>(masterPtr);
    ASSERT_TRUE(rtuMasterConfig);

    EXPECT_EQ(rtuMasterConfig->GetFrameType(), modbus::RTU);
    EXPECT_STREQ(rtuMasterConfig->device.c_str(), "/dev/ttyUSB2");
    EXPECT_EQ(rtuMasterConfig->timeout.count(), 3214);
    EXPECT_EQ(rtuMasterConfig->rtuOptions.baudRate.value(), asio::serial_port_base::baud_rate().value());
    EXPECT_EQ(rtuMasterConfig->rtuOptions.characterSize.value(), asio::serial_port_base::character_size().value());
    EXPECT_EQ(rtuMasterConfig->rtuOptions.parity.value(), asio::serial_port_base::parity().value());
    EXPECT_EQ(rtuMasterConfig->rtuOptions.stopBits.value(), asio::serial_port_base::stop_bits().value());
    EXPECT_EQ(rtuMasterConfig->rtuOptions.flowControl.value(), asio::serial_port_base::flow_control().value());
    EXPECT_EQ(rtuMasterConfig->unitIdSet.size(), 0);
  }
}

TEST(ConfigTest, UnitIdTest) {
  std::stringstream is;
  is << R"(
{
  "unit_id1": {
    "type": "range",
    "begin": 10,
    "end": 20
  },
  "unit_id2": {
    "type": "value",
    "value": 3
  }
}
)";
  auto data = nlohmann::json::parse(is);
  modbus_gateway::TracePath tp;
  {
    modbus_gateway::TraceDeep td(tp, "unit_id1");

    auto obj = modbus_gateway::FindObject(td, data);

    auto unitIdRange = modbus_gateway::UnitIdRange(td.GetTracePath(), obj);

    EXPECT_EQ(unitIdRange.begin,10);
    EXPECT_EQ(unitIdRange.end,20);
  }
  {
    modbus_gateway::TraceDeep td(tp, "unit_id2");

    auto obj = modbus_gateway::FindObject(td, data);

    auto unitIdRange = modbus_gateway::UnitIdRange(td.GetTracePath(), obj);

    EXPECT_EQ(unitIdRange.begin,3);
    EXPECT_EQ(unitIdRange.end,3);
  }
}
