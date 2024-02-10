#include <common/modbus_message_actor.h>

#include <common/logger.h>
#include <exchange/exchange.h>

namespace test {

ModbusMessageActor::ModbusMessageActor(const exchange::ExchangePtr &exchange)
    : id_(exchange::defaultId), exchange_(exchange), handler_([](const auto &ptr) { return ptr; }) {}

void ModbusMessageActor::Receive(const exchange::MessagePtr &message) {
  MG_TRACE("ModbusMessageActor({})::Receive message", id_);
  const modbus_gateway::ModbusMessagePtr &modbusMessage = std::dynamic_pointer_cast<modbus_gateway::ModbusMessagePtr::element_type>(
      message);

  if (modbusMessage) {
    const exchange::ActorId sourceId = modbusMessage->GetModbusMessageInfo().GetSourceId();
    MG_DEBUG("ModbusMessageActor({})::Receive modbus message from {}", id_, sourceId);
    modbus_gateway::ModbusMessagePtr response = handler_(modbusMessage);
    auto exchange = exchange_.lock();
    if (exchange) {
      exchange->Send(sourceId, response);
    }
    return;
  }
  MG_CRIT("ModbusMessageActor({})::Receive unknown message type", id_);
}

void ModbusMessageActor::SetId(exchange::ActorId id) {
  id_ = id;
}

void ModbusMessageActor::ResetId() {
  id_ = exchange::defaultId;
}

exchange::ActorId ModbusMessageActor::GetId() {
  return id_;
}

void ModbusMessageActor::SetHandler(const ModbusMessageActor::Handler &handler) {
  handler_ = handler;
}

}// namespace test
