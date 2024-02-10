#include <transport/modbus_rtu_slave.h>

#include <common/logger.h>

#include <modbus/modbus_buffer.h>
#include <modbus/modbus_buffer_wrapper.h>

namespace modbus_gateway {

ModbusRtuSlave::ModbusRtuSlave(const exchange::ExchangePtr &exchange,
                               const ContextPtr &context,
                               const std::string &device,
                               const RtuOptions &options,
                               const RouterPtr &router,
                               modbus::FrameType frameType)
    : IModbusSlave(TransportType::RtuSlave),
      id_(exchange::defaultId),
      exchange_(exchange),
      serialPort_(*context),
      router_(router),
      frameType_(frameType),
      idGenerator_(0),
      syncRequestInfo_(std::nullopt) {
  serialPort_.open(device);
  serialPort_.set_option(options.baudRate);
  serialPort_.set_option(options.characterSize);
  serialPort_.set_option(options.parity);
  serialPort_.set_option(options.stopBits);
  serialPort_.set_option(options.flowControl);

  assert(exchange);
  assert(router);
  assert(frameType_ != modbus::TCP);

  MG_DEBUG("ModbusRtuSlave({})::Ctor: {}", id_, device);
}

ModbusRtuSlave::~ModbusRtuSlave() {
  MG_DEBUG("ModbusRtuSlave({})::Dtor", id_);
  Stop();
  asio::error_code ec;
  ec = serialPort_.close(ec);
  if (ec) {
    MG_WARN("ModbusRtuSlave({})::Dtor serial port close error: {}", id_, ec.message());
  }
}

void ModbusRtuSlave::Receive(const exchange::MessagePtr &message) {
  const ModbusMessagePtr &modbusMessage = std::dynamic_pointer_cast<ModbusMessagePtr::element_type>(message);
  if (modbusMessage) {
    MG_TRACE("ModbusRtuSlave({})::Receive: ModbusMessage", id_);
    StartWriteTask(modbusMessage);
    return;
  }
  MG_WARN("ModbusRtuSlave({})::Receive: unsupported message", id_);
}

void ModbusRtuSlave::SetId(exchange::ActorId id) {
  id_ = id;
}

void ModbusRtuSlave::ResetId() {
  id_ = exchange::defaultId;
}

exchange::ActorId ModbusRtuSlave::GetId() {
  return id_;
}

void ModbusRtuSlave::Start() {
  MG_DEBUG("ModbusRtuSlave({})::Start", id_);
  StartReadTask();
}

void ModbusRtuSlave::Stop() {
  MG_DEBUG("ModbusRtuSlave({})::Stop", id_);
  asio::error_code ec;
  ec = serialPort_.cancel(ec);
  if (ec) {
    MG_WARN("ModbusRtuSlave({})::Stop serial port cancel error: {}", id_, ec.message());
  }
}

void ModbusRtuSlave::StartReadTask() {
  MG_TRACE("ModbusRtuSlave({})::StartReadTask", id_);
  Weak weak = GetWeak();
  auto modbusBuffer = std::make_shared<modbus::ModbusBuffer>(frameType_);
  serialPort_.async_read_some(asio::buffer(modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize()),
                              [weak, modbusBuffer](asio::error_code ec, size_t size) {
                                Ptr self = weak.lock();
                                if (!self) {
                                  MG_WARN("ModbusRtuSlave::read: actor was deleted");
                                  return;
                                }

                                auto exchange = self->exchange_.lock();
                                if (!exchange) {
                                  MG_WARN("ModbusRtuSlave({})::accept: exchange was deleted");
                                  return;
                                }

                                if (ec) {
                                  if ((asio::error::eof == ec)
                                      || (asio::error::connection_reset == ec)
                                      || (asio::error::operation_aborted == ec)) {
                                    MG_INFO("ModbusRtuSlave({})::read: cancelled", self->id_);
                                    return;
                                  }
                                  MG_ERROR("ModbusRtuSlave({})::read: error: {}", self->id_, ec.message());
                                  self->StartReadTask();
                                  return;
                                }

                                MG_TRACE("ModbusRtuSlave({})::read: {} bytes", self->id_, size);
                                auto message = self->MakeRequest(modbusBuffer, size);
                                if (!message) {
                                  MG_ERROR("ModbusRtuSlave: receive: invalid request, start receive task");
                                  self->StartReadTask();
                                  return;
                                }

                                {
                                  auto access = self->syncRequestInfo_.GetAccess();
                                  access.ref = message->GetModbusMessageInfo();
                                }

                                const modbus::UnitId unitId = message->GetModbusBuffer()->GetUnitId();
                                const exchange::ActorId actorId = self->router_->Route(unitId);
                                MG_DEBUG("ModbusRtuSlave({})::read: unit id {} route to actor id {}",
                                         self->id_, unitId, actorId);
                                const auto res = exchange->Send(actorId, message);
                                if (!res) {
                                  MG_ERROR("ModbusRtuSlave({})::read: route to actor id {} failed",
                                           self->id_, actorId);
                                }
                                self->StartReadTask();
                              });
}

ModbusMessagePtr ModbusRtuSlave::MakeRequest(const ModbusBufferPtr &modbusBuffer, size_t size) {
  if (!modbusBuffer->SetAduSize(size)) {
    MG_ERROR("ModbusRtuSlave({})::MakeRequest: invalid adu size", id_);
    return nullptr;
  }
  MG_TRACE("ModbusRtuSlave({})::MakeRequest: request: [{:X}]", id_, fmt::join(*modbusBuffer, " "))

  const auto wrapper = modbus::MakeModbusBufferWrapper(*modbusBuffer);
  const auto checkFrameResult = wrapper->Check();
  if (checkFrameResult != modbus::CheckFrameResult::NoError) {
    MG_ERROR("ModbusRtuSlave({})::MakeRequest: invalid frame {}", id_, checkFrameResult);
    return nullptr;
  }

  // Generate transaction id for identification response
  const auto transactionId = GetNextId();

  MG_DEBUG("ModbusRtuSlave({})::MakeRequest: transaction id {}", id_, transactionId);

  ModbusMessageInfo modbusMessageInfo(id_, transactionId);
  return std::make_shared<ModbusMessagePtr::element_type>(modbusMessageInfo, modbusBuffer);
}

modbus::TransactionId ModbusRtuSlave::GetNextId() {
  return ++idGenerator_;
}

void ModbusRtuSlave::StartWriteTask(const ModbusMessagePtr &modbusMessage) {
  MG_TRACE("ModbusRtuSlave({})::StartWriteTask", id_);
  ModbusBufferPtr modbusBuffer = MakeResponse(modbusMessage);
  if (!modbusBuffer) {
    return;
  }

  Weak weak = GetWeak();
  serialPort_.async_write_some(asio::buffer(modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize()),
                               [weak, modbusBuffer](asio::error_code ec, size_t size) {
                                 Ptr self = weak.lock();
                                 if (!self) {
                                   MG_WARN("ModbusRtuSlave::write: actor was deleted");
                                   return;
                                 }

                                 if (ec) {
                                   MG_WARN("ModbusRtuSlave({})::write: error: {}", self->id_, ec.message());
                                   return;
                                 }

                                 MG_TRACE("ModbusRtuSlave({})::write: {} bytes", self->id_, size);
                               });
}

ModbusBufferPtr ModbusRtuSlave::MakeResponse(const ModbusMessagePtr &modbusMessage) {
  const ModbusMessageInfo &messageInfo = modbusMessage->GetModbusMessageInfo();
  ModbusBufferPtr modbusBuffer = modbusMessage->GetModbusBuffer();

  if (id_ != messageInfo.GetSourceId()) {
    MG_CRIT("ModbusRtuSlave({})::MakeResponse: invalid message source id {}",
            id_,
            messageInfo.GetSourceId());
    return nullptr;
  }

  {
    auto access = syncRequestInfo_.GetAccess();
    ModbusMessageInfoOpt &modbusMessageInfoOpt = access.ref;
    if (!modbusMessageInfoOpt.has_value()) {
      MG_ERROR(
          "ModbusRtuSlave({})::MakeResponse: last message info is empty, message transaction id {}",
          id_,
          messageInfo.GetTransactionId());
      return nullptr;
    }
    const ModbusMessageInfo &lastInfo = modbusMessageInfoOpt.value();
    if (lastInfo.GetTransactionId() != messageInfo.GetTransactionId()) {
      MG_ERROR(
          "ModbusRtuSlave({})::MakeResponse: message transaction id {} not equal last transaction id {}",
          id_,
          messageInfo.GetTransactionId(), lastInfo.GetTransactionId());
      return nullptr;
    }
    modbusMessageInfoOpt.reset();
  }

  if (!modbusBuffer) {
    MG_CRIT("ModbusRtuSlave({})::MakeResponse: modbus buffer is null. transaction id {}", id_,
            messageInfo.GetTransactionId());
    return nullptr;
  }

  const auto originType = modbusBuffer->GetType();
  modbusBuffer->ConvertTo(frameType_);

  auto wrapper = modbus::MakeModbusBufferWrapper(*modbusBuffer);
  wrapper->Update();

  MG_DEBUG("ModbusRtuSlave({})::MakeResponse: frame type {}->{}, transaction Id {}",
           id_,
           static_cast<int>(originType),
           static_cast<int>(frameType_),
           static_cast<int>(messageInfo.GetTransactionId()));
  MG_DEBUG("ModbusRtuSlave({})::MakeResponse: response: [{:X}]", id_, fmt::join(*modbusBuffer, " "));

  return modbusBuffer;
}

}// namespace modbus_gateway