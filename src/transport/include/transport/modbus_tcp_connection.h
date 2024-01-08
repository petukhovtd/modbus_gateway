#pragma once

#include <common/synchronized.h>
#include <common/types_asio.h>
#include <message/modbus_message.h>
#include <message/modbus_message_info.h>
#include <transport/irouter.h>

#include <exchange/actor_helper.h>
#include <exchange/iexchange.h>

#include <optional>

namespace modbus_gateway {

class ModbusTcpConnection final : public exchange::ActorHelper<ModbusTcpConnection> {
  using ModbusMessageInfoOpt = std::optional<ModbusMessageInfo>;

public:
  ModbusTcpConnection(const exchange::ExchangePtr &exchange, exchange::ActorId serverId,
                      TcpSocketPtr socket, const RouterPtr &router);

  ~ModbusTcpConnection() override;

  void Receive(const exchange::MessagePtr &) override;

  void SetId(exchange::ActorId id) override;

  void ResetId() override;

  exchange::ActorId GetId() override;

  void Start();

  void Stop();

private:
  static ModbusMessagePtr
  MakeRequest(const ModbusBufferPtr &modbusBuffer, size_t size, exchange::ActorId masterId);

  void StartReceiveTask();

  ModbusBufferPtr MakeResponse(const ModbusMessagePtr &modbusMessage);

  void StartSendTask(const ModbusMessagePtr &modbusMessage);

private:
  std::atomic<exchange::ActorId> id_;
  exchange::ExchangePtr exchange_;
  exchange::ActorId serverId_;
  TcpSocketPtr socket_;
  RouterPtr router_;
  Synchronized<ModbusMessageInfoOpt> syncRequestInfo_;
};

}// namespace modbus_gateway
