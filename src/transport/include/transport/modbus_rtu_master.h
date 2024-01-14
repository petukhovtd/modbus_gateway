#pragma once

#include <common/types_modbus.h>
#include <common/limit_queue.h>
#include <transport/rtu_options.h>
#include <message/modbus_message.h>

#include <exchange/actor_helper.h>
#include <exchange/iexchange.h>

#include <modbus/modbus_buffer.h>

#include <optional>

namespace modbus_gateway {

class ModbusRtuMaster : public exchange::ActorHelper<ModbusRtuMaster> {

  struct ModbusCurrentMessage {
    ModbusMessagePtr modbusMessage;
    modbus::TransactionId id;
  };

public:
  ModbusRtuMaster(const exchange::ExchangePtr &exchange,
                  const ContextPtr &context,
                  const std::string &device,
                  const RtuOptions &options,
                  std::chrono::milliseconds timeout,
                  modbus::FrameType frameType);

  ~ModbusRtuMaster() override;

  void Receive(const exchange::MessagePtr &message) override;

  void SetId(exchange::ActorId id) override;

  void ResetId() override;

  exchange::ActorId GetId() override;

private:
  void MessageProcess(const ModbusMessagePtr &message);

  void QueueProcessUnsafe();

  void StartMessageTaskUnsafe();

  void StartWaitTask();

  void StartReadTask();

  ModbusMessagePtr MakeResponse(const ModbusBufferPtr &modbusBuffer, size_t size);

private:
  exchange::ActorId id_;
  exchange::ExchangeWeak exchange_;
  asio::serial_port serialPort_;
  std::chrono::milliseconds timeout_;
  modbus::FrameType frameType_;
  asio::basic_waitable_timer<std::chrono::steady_clock> timer_;
  std::mutex m_;
  LimitQueue<ModbusMessagePtr> messageQueue_;
  std::optional<ModbusCurrentMessage> currentMessage_;
  modbus::TransactionId transactionIdGenerator_;
};

}// namespace modbus_gateway
