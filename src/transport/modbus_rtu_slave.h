#pragma once

#include <common/types_asio.h>
#include <common/types_modbus.h>
#include <common/synchronized.h>
#include <transport/irouter.h>
#include <transport/rtu_options.h>
#include <message/modbus_message.h>
#include <message/modbus_message_info.h>

#include <exchange/actor_helper.h>
#include <exchange/iexchange.h>

#include <modbus/modbus_types.h>
#include <modbus/modbus_buffer.h>
#include <modbus/modbus_buffer_wrapper.h>

namespace modbus_gateway {

class ModbusRtuSlave : public exchange::ActorHelper<ModbusRtuSlave> {
  using ModbusMessageInfoOpt = std::optional<ModbusMessageInfo>;
public:

  ModbusRtuSlave(const exchange::ExchangePtr &exchange,
                  const ContextPtr &context,
                  const std::string &device,
                  const RtuOptions &options,
                  const RouterPtr &router,
                  modbus::FrameType frameType);

  ~ModbusRtuSlave() override;

  void Receive(const exchange::MessagePtr &message) override;

  void SetId(exchange::ActorId id) override;

  void ResetId() override;

  void Start();

  void Stop();

private:
  void StartReadTask();

  ModbusMessagePtr MakeRequest(const ModbusBufferPtr &modbusBuffer, size_t size);

  modbus::TransactionId GetNextId();

  void StartWriteTask(const ModbusMessagePtr &modbusMessage);

  ModbusBufferPtr MakeResponse(const ModbusMessagePtr &modbusMessage);

  std::unique_ptr<modbus::IModbusBufferWrapper> MakeWrapper(modbus::ModbusBuffer &modbusBuffer);

private:
  exchange::ActorId id_;
  exchange::ExchangePtr exchange_;
  asio::serial_port serialPort_;
  RouterPtr router_;
  modbus::FrameType frameType_;
  modbus::TransactionId idGenerator_;
  Synchronized<ModbusMessageInfoOpt> syncRequestInfo_;
};

}// namespace modbus_gateway
