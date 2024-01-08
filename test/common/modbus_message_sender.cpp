#include <common/modbus_message_sender.h>

#include <common/logger.h>

namespace test {
ModbusMessageSender::ModbusMessageSender(const exchange::ExchangePtr &exchange)
    : id_(exchange::defaultId),
      exchange_(exchange),
      transactionIdGenerator_(0) {
}

void ModbusMessageSender::Receive(const exchange::MessagePtr &message) {
  MG_TRACE("ModbusMessageSender({})::Receive message", id_);
  const modbus_gateway::ModbusMessagePtr &modbusMessage = std::dynamic_pointer_cast<modbus_gateway::ModbusMessagePtr::element_type>(
      message);

  if (modbusMessage) {
    const exchange::ActorId sourceId = modbusMessage->GetModbusMessageInfo().GetSourceId();
    MG_DEBUG("ModbusMessageSender({})::Receive modbus message from {}", id_, sourceId);
    receiveMessage = modbusMessage;
    return;
  }
  MG_CRIT("ModbusMessageSender({})::Receive unknown message type", id_);
}

void ModbusMessageSender::SetId(exchange::ActorId id) {
  id_ = id;
}

void ModbusMessageSender::ResetId() {
  id_ = exchange::defaultId;
}

void ModbusMessageSender::SendTo(const modbus_gateway::ModbusBufferPtr &modbusBuffer, exchange::ActorId target) {
  const auto transactionId = ++transactionIdGenerator_;
  const auto modbusMessageInfo = modbus_gateway::ModbusMessageInfo{id_, transactionId};
  auto modbusMessage = modbus_gateway::ModbusMessage::Create(modbusMessageInfo, modbusBuffer);
  MG_INFO("ModbusMessageSender({})::SendTo: transactionId {}, targetId {}", id_, transactionId, target);
  exchange_->Send(target, modbusMessage);
}

}// namespace test