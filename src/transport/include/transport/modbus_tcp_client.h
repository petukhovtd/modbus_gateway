#pragma once

#include <common/limit_queue.h>
#include <common/types_asio.h>
#include <message/modbus_message.h>

#include <exchange/actor_helper.h>
#include <exchange/iexchange.h>

namespace modbus_gateway {

class ModbusTcpClient : public exchange::ActorHelper<ModbusTcpClient> {

  struct ModbusCurrentMessage {
    ModbusMessagePtr modbusMessage;
    modbus::TransactionId id;
  };

  enum class State {
    Idle,
    WaitConnect,
    Connected,
    MessageProcess,
  };

public:
  ModbusTcpClient(const exchange::ExchangePtr &exchange,
                  const ContextPtr &context,
                  const asio::ip::address &addr,
                  asio::ip::port_type port,
                  std::chrono::milliseconds timeout);

  ~ModbusTcpClient() override;

  void Receive(const exchange::MessagePtr &message) override;

  void SetId(exchange::ActorId id) override;

  void ResetId() override;

  exchange::ActorId GetId() override;

private:
  void MessageProcess(const ModbusMessagePtr &message);

  void QueueProcessUnsafe();

  void StartConnectTaskUnsafe();

  void StartMessageTaskUnsafe();

  void StartWaitTask();

  ModbusMessagePtr MakeResponse(const ModbusBufferPtr &modbusBuffer, size_t size);

  void StartReceiveTask();

  static std::string StateToStr(State state);

  void CloseSocket();

private:
  exchange::ActorId id_;
  exchange::ExchangePtr exchange_;
  TcpSocketPtr socket_;
  asio::ip::tcp::endpoint ep_;
  std::chrono::milliseconds timeout_;
  asio::basic_waitable_timer<std::chrono::steady_clock> timer_;
  std::mutex m_;
  LimitQueue<ModbusMessagePtr> messageQueue_;
  std::optional<ModbusCurrentMessage> currentMessage_;
  modbus::TransactionId transactionIdGenerator_;
  State state_;
};

}// namespace modbus_gateway
