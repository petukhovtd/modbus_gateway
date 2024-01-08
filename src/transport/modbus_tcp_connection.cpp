#include <transport/modbus_tcp_connection.h>

#include <common/logger.h>
#include <message/client_disconnect_message.h>
#include <message/modbus_message.h>

#include <modbus/modbus_buffer.h>
#include <modbus/modbus_buffer_tcp_wrapper.h>

using namespace asio;

namespace modbus_gateway {

ModbusTcpConnection::ModbusTcpConnection(const exchange::ExchangePtr &exchange, exchange::ActorId serverId,
                                         TcpSocketPtr socket, const RouterPtr &router)
    : id_(exchange::defaultId), exchange_(exchange), serverId_(serverId), socket_(std::move(socket)), router_(router),
      syncRequestInfo_(std::nullopt) {
  assert(exchange_);
  assert(socket_);
  assert(router_);
  MG_TRACE("ModbusTcpConnection({})::Ctor: serverId {}", id_, serverId_);
}

ModbusTcpConnection::~ModbusTcpConnection() {
  MG_TRACE("ModbusTcpConnection({})::Dtor", id_);
  Stop();
  error_code ec;
  ec = socket_->shutdown( socket_base::shutdown_both, ec );
  if (ec) {
    MG_WARN("ModbusTcpConnection({})::Dtor: socket shutdown error: {}", id_, ec.message());
  }
  ec = socket_->close(ec);
  if (ec) {
    MG_WARN("ModbusTcpConnection({})::Dtor: socket close error: {}", id_, ec.message());
  }
}

void ModbusTcpConnection::Receive(const exchange::MessagePtr &message) {
  MG_TRACE("ModbusTcpConnection({})::Receive", id_);
  const ModbusMessagePtr &modbusMessage = std::dynamic_pointer_cast<ModbusMessagePtr::element_type>(message);
  if (modbusMessage) {
    MG_TRACE("ModbusTcpConnection({})::Receive: ModbusMessage", id_);
    StartSendTask(modbusMessage);
    return;
  }
  MG_WARN("ModbusTcpConnection({})::Receive: unsupported message", id_);
}

void ModbusTcpConnection::SetId(exchange::ActorId id) {
  id_ = id;
}

void ModbusTcpConnection::ResetId() {
  id_ = exchange::defaultId;
}

exchange::ActorId ModbusTcpConnection::GetId() {
  return id_;
}

void ModbusTcpConnection::Start() {
  assert(id_ != exchange::defaultId);
  MG_INFO("ModbusTcpConnection({})::Start: serverId {}, client {}:{}",
          id_, serverId_,
          socket_->remote_endpoint().address().to_string(),
          socket_->remote_endpoint().port())
  StartReceiveTask();
}

void ModbusTcpConnection::Stop() {
  MG_INFO("ModbusTcpConnection({})::Stop", id_);
  error_code ec;
  ec = socket_->cancel(ec);
  if (ec) {
    MG_WARN("ModbusTcpConnection({})::Stop socket cancel error: {}", id_, ec.message());
  }
}

ModbusMessagePtr
ModbusTcpConnection::MakeRequest(const ModbusBufferPtr &modbusBuffer, size_t size, exchange::ActorId masterId) {
  if (!modbusBuffer->SetAduSize(size)) {
    MG_ERROR("ModbusTcpConnection({})::MakeRequest: invalid adu size", masterId);
    return nullptr;
  }
  MG_TRACE("ModbusTcpConnection({})::MakeRequest: request: [{:X}]", masterId, fmt::join(*modbusBuffer, " "))
  modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper(*modbusBuffer);
  modbus::CheckFrameResult checkFrameResult = modbusBufferTcpWrapper.Check();
  if (checkFrameResult != modbus::CheckFrameResult::NoError) {
    MG_ERROR("ModbusTcpConnection({})::MakeRequest: invalid tcp frame {}", masterId, checkFrameResult);
    return nullptr;
  }

  MG_DEBUG("ModbusTcpConnection({})::MakeRequest: request: transaction id {}, "
           "protocol id {}, "
           "length {}, "
           "unit id {}, "
           "function code {}",
           masterId,
           modbusBufferTcpWrapper.GetTransactionId(),
           modbusBufferTcpWrapper.GetProtocolId(),
           modbusBufferTcpWrapper.GetLength(),
           modbusBuffer->GetUnitId(),
           modbusBuffer->GetFunctionCode());

  ModbusMessageInfo modbusMessageInfo(masterId, modbusBufferTcpWrapper.GetTransactionId());
  return std::make_shared<ModbusMessagePtr::element_type>(modbusMessageInfo, modbusBuffer);
}

void ModbusTcpConnection::StartReceiveTask() {
  MG_TRACE("ModbusTcpConnection({})::StartReadTask", id_);
  Weak weak = GetWeak();
  auto modbusBuffer = std::make_shared<modbus::ModbusBuffer>(modbus::FrameType::TCP);
  socket_->async_receive(buffer(modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize()),
                         [weak, modbusBuffer](error_code ec, size_t size) {
                           Ptr self = weak.lock();
                           if (!self) {
                             MG_CRIT("ModbusTcpConnection::receive: actor was deleted");
                             return;
                           }

                           if (ec) {
                             MG_DEBUG("ModbusTcpConnection({})::receive: error: {}", self->id_,
                                      ec.message())
                             if ((error::eof == ec) || (error::connection_reset == ec) || (error::operation_aborted == ec)) {
                               MG_INFO("ModbusTcpConnection({})::receive: send disconnect message",
                                       self->id_);
                               self->exchange_->Send(self->serverId_,
                                                     ClientDisconnectMessage::Create(self->id_));
                               return;
                             }
                             MG_ERROR("ModbusTcpConnection({})::receive: error: {}", self->id_,
                                      ec.message())
                             MG_TRACE("ModbusTcpConnection({})::receive: start receive task",
                                      self->id_);
                             self->StartReceiveTask();
                             return;
                           }

                           MG_DEBUG("ModbusTcpConnection({})::receive: {} bytes", self->id_, size);
                           auto message = self->MakeRequest(modbusBuffer, size, self->id_);
                           if (!message) {
                             MG_ERROR("ModbusTcpConnection: receive: invalid request, start receive task");
                             self->StartReceiveTask();
                             return;
                           }

                           {
                             MG_TRACE("ModbusTcpConnection({})::receive: update modbus message info",
                                      self->id_);
                             auto access = self->syncRequestInfo_.GetAccess();
                             access.ref = message->GetModbusMessageInfo();
                           }

                           const modbus::UnitId unitId = message->GetModbusBuffer()->GetUnitId();
                           const exchange::ActorId slaveId = self->router_->Route(unitId);
                           MG_TRACE("ModbusTcpConnection({})::receive: unit id {} route to slave id {}",
                                    self->id_, message->GetModbusMessageInfo().GetSourceId(), slaveId);
                           const auto res = self->exchange_->Send(slaveId, message);
                           if (!res) {
                             MG_ERROR("ModbusTcpConnection({})::receive: route to slave id {} failed",
                                      self->id_, slaveId);
                           }
                           MG_TRACE("ModbusTcpConnection({})::receive: start receive task", self->id_);
                           self->StartReceiveTask();
                         });
}

ModbusBufferPtr ModbusTcpConnection::MakeResponse(const ModbusMessagePtr &modbusMessage) {
  MG_TRACE("ModbusTcpConnection({})::MakeResponse", id_);

  const ModbusMessageInfo &messageInfo = modbusMessage->GetModbusMessageInfo();
  ModbusBufferPtr modbusBuffer = modbusMessage->GetModbusBuffer();

  MG_TRACE("ModbusTcpConnection({})::MakeResponse: message source id {}, transaction id {}", id_,
           messageInfo.GetSourceId(), messageInfo.GetTransactionId());

  if (id_ != messageInfo.GetSourceId()) {
    MG_CRIT("ModbusTcpConnection({})::MakeResponse: invalid message source id {}",
            id_,
            messageInfo.GetSourceId());
    return nullptr;
  }

  {
    auto access = syncRequestInfo_.GetAccess();
    ModbusMessageInfoOpt &modbusMessageInfoOpt = access.ref;
    if (!modbusMessageInfoOpt.has_value()) {
      MG_ERROR(
          "ModbusTcpConnection({})::MakeResponse: last message info is empty, message transaction id {}",
          id_,
          messageInfo.GetTransactionId());
      return nullptr;
    }
    const ModbusMessageInfo &lastInfo = modbusMessageInfoOpt.value();
    if (lastInfo.GetTransactionId() != messageInfo.GetTransactionId()) {
      MG_ERROR(
          "ModbusTcpConnection({})::MakeResponse: message transaction id {} not equal last transaction id {}",
          id_,
          messageInfo.GetTransactionId(), lastInfo.GetTransactionId());
      return nullptr;
    }
    MG_TRACE("ModbusTcpConnection({})::MakeResponse: message info checking successfully", id_);
    modbusMessageInfoOpt.reset();
  }

  if (!modbusBuffer) {
    MG_CRIT("ModbusTcpConnection({})::MakeResponse: modbus buffer is null. transaction id {}", id_,
            messageInfo.GetTransactionId());
    return nullptr;
  }

  modbusBuffer->ConvertTo(modbus::FrameType::TCP);
  modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper(*modbusBuffer);
  modbusBufferTcpWrapper.Update();
  modbusBufferTcpWrapper.SetTransactionId(messageInfo.GetTransactionId());

  MG_DEBUG("ModbusTcpConnection({})::MakeResponse: response: transaction id {}, "
           "protocol id {}, "
           "length {}, "
           "unit id {}, "
           "function code {}",
           id_,
           modbusBufferTcpWrapper.GetTransactionId(),
           modbusBufferTcpWrapper.GetProtocolId(),
           modbusBufferTcpWrapper.GetLength(),
           modbusBuffer->GetUnitId(),
           modbusBuffer->GetFunctionCode());

  MG_TRACE("ModbusTcpConnection({})::MakeResponse: response: [{:X}]", id_, fmt::join(*modbusBuffer, " "));

  return modbusBuffer;
}

void ModbusTcpConnection::StartSendTask(const ModbusMessagePtr &modbusMessage) {
  MG_TRACE("ModbusTcpConnection({})::StartSendTask", id_);
  ModbusBufferPtr modbusBuffer = MakeResponse(modbusMessage);
  if (!modbusBuffer) {
    return;
  }

  Weak weak = GetWeak();
  socket_->async_send(buffer(modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize()),
                      [weak, modbusBuffer](error_code ec, size_t size) {
                        Ptr self = weak.lock();
                        if (!self) {
                          MG_CRIT("ModbusTcpConnection::send: actor was deleted");
                          return;
                        }

                        if (ec) {
                          MG_ERROR("ModbusTcpConnection({})::send: error: {}", self->id_, ec.message());
                          return;
                        }

                        MG_DEBUG("ModbusTcpConnection({})::send: {} bytes", self->id_, size);
                      });
}

}// namespace modbus_gateway
