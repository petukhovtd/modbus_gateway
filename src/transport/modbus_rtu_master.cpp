#include <transport/modbus_rtu_master.h>

#include <common/logger.h>

#include <modbus/modbus_buffer_wrapper.h>

namespace modbus_gateway {

ModbusRtuMaster::ModbusRtuMaster(const exchange::ExchangePtr &exchange,
                                 const ContextPtr &context,
                                 const std::string &device,
                                 const RtuOptions &options,
                                 std::chrono::milliseconds timeout,
                                 modbus::FrameType frameType)
    : id_(exchange::defaultId),
      exchange_(exchange),
      serialPort_(*context),
      timeout_(timeout),
      frameType_(frameType),
      timer_(serialPort_.get_executor()),
      m_(), messageQueue_(),
      currentMessage_(std::nullopt),
      transactionIdGenerator_(0) {
  serialPort_.open(device);
  serialPort_.set_option(options.baudRate);
  serialPort_.set_option(options.characterSize);
  serialPort_.set_option(options.parity);
  serialPort_.set_option(options.stopBits);
  serialPort_.set_option(options.flowControl);

  assert(exchange);
  assert(frameType_ != modbus::TCP);

  MG_TRACE("ModbusRtuMaster::Ctor: {}", device);
}

ModbusRtuMaster::~ModbusRtuMaster() {
  asio::error_code ec;
  ec = serialPort_.cancel(ec);
  if (ec) {
    MG_WARN("ModbusRtuMaster({})::Dtor serial port cancel error: {}", id_, ec.message());
  }
  ec = serialPort_.close(ec);
  if (ec) {
    MG_WARN("ModbusRtuMaster({})::Dtor serial port close error: {}", id_, ec.message());
  }
}

void ModbusRtuMaster::Receive(const exchange::MessagePtr &message) {
  MG_TRACE("ModbusRtuMaster({})::Receive", id_);
  auto modbusMessage = std::dynamic_pointer_cast<ModbusMessagePtr::element_type>(message);
  if (modbusMessage) {
    MG_TRACE("ModbusRtuMaster({})::Receive: ModbusMessage", id_);
    MessageProcess(modbusMessage);
    return;
  }
  MG_WARN("ModbusRtuMaster({})::Receive: unsupported message", id_);
}

void ModbusRtuMaster::SetId(exchange::ActorId id) {
  id_ = id;
}

void ModbusRtuMaster::ResetId() {
  id_ = exchange::defaultId;
}

exchange::ActorId ModbusRtuMaster::GetId() {
  return 0;
}

void ModbusRtuMaster::MessageProcess(const ModbusMessagePtr &message) {
  std::lock_guard<std::mutex> lock(m_);
  messageQueue_.Push(message);
  MG_TRACE("ModbusRtuMaster({})::MessageProcess: message in queue {}", id_, messageQueue_.Size());

  QueueProcessUnsafe();
}

void ModbusRtuMaster::QueueProcessUnsafe() {
  MG_TRACE("ModbusRtuMaster({})::QueueProcess", id_);
  if (messageQueue_.Empty()) {
    MG_DEBUG("ModbusRtuMaster({})::QueueProcess: queue is empty", id_);
    return;
  }

  if (currentMessage_) {
    MG_INFO("ModbusRtuMaster({})::QueueProcess: message in process", id_);
    return;
  }

  while (!messageQueue_.Empty()) {
    const auto &message = messageQueue_.Front();
    if (!message->GetModbusMessageInfo().TimeoutReached(timeout_)) {
      break;
    }

    const auto &messageInfo = message->GetModbusMessageInfo();
    MG_INFO(
        "ModbusRtuMaster({})::QueueProcess: message reached timeout {}, message id {}, source id {}", id_,
        timeout_.count(), messageInfo.GetTransactionId(), messageInfo.GetSourceId());
    messageQueue_.Pop();
  }

  if (messageQueue_.Empty()) {
    MG_INFO("ModbusRtuMaster({})::QueueProcess: message queue empty", id_);
    return;
  }

  StartMessageTaskUnsafe();
}
void ModbusRtuMaster::StartMessageTaskUnsafe() {
  MG_TRACE("ModbusRtuMaster({})::StartMessageTaskUnsafe", id_);

  const auto transactionId = ++transactionIdGenerator_;
  currentMessage_ = {messageQueue_.Front(), transactionId};
  messageQueue_.Pop();

  ModbusBufferPtr modbusBuffer = currentMessage_->modbusMessage->GetModbusBuffer();
  modbusBuffer->ConvertTo(frameType_);
  {
    auto wrapper = modbus::MakeModbusBufferWrapper(*modbusBuffer);
    wrapper->Update();

    MG_DEBUG("ModbusRtuMaster({})::StartMessageTask: request: transaction id {}, "
             "unit id {}, "
             "function code {}",
             id_,
             transactionId,
             modbusBuffer->GetUnitId(),
             modbusBuffer->GetFunctionCode())

    MG_DEBUG("ModbusRtuMaster({})::StartMessageTask: request: [{:X}]", id_, fmt::join(*modbusBuffer, " "));
  }

  Weak weak = GetWeak();
  serialPort_.async_write_some(asio::buffer(modbusBuffer->begin().base(), modbusBuffer->GetAduSize()),
                               [weak](asio::error_code ec, size_t size) {
                                 Ptr self = weak.lock();
                                 if (!self) {
                                   MG_CRIT("ModbusRtuMaster::write: actor was deleted")
                                   return;
                                 }

                                 std::lock_guard<std::mutex> lock(self->m_);

                                 if (ec) {
                                   MG_ERROR("ModbusRtuMaster({})::write: error {}", self->id_, ec.message());
                                   self->currentMessage_.reset();
                                   return;
                                 }

                                 MG_TRACE("ModbusRtuMaster({})::write: start wait task", self->id_);
                                 self->StartWaitTask();
                                 MG_TRACE("ModbusRtuMaster({})::write: start read task", self->id_);
                                 self->StartReadTask();
                               });
}

void ModbusRtuMaster::StartWaitTask() {
  MG_TRACE("ModbusRtuMaster({})::StartWaitTask", id_);

  MG_DEBUG("ModbusRtuMaster({})::StartWaitTask timeout {}ms", id_, timeout_.count())
  timer_.expires_after(timeout_);

  Weak weak = GetWeak();
  timer_.async_wait([weak](asio::error_code ec) {
    Ptr self = weak.lock();
    if (!self) {
      MG_CRIT("ModbusRtuMaster::wait: actor was deleted")
      return;
    }

    std::lock_guard<std::mutex> lock(self->m_);

    if (ec) {
      if (asio::error::operation_aborted == ec) {
        MG_TRACE("ModbusRtuMaster({})::wait: canceled", self->id_);
        return;
      }
      MG_ERROR("ModbusRtuMaster({})::wait: error: {}", self->id_, ec.message())
      return;
    }

    MG_TRACE("ModbusRtuMaster({})::wait: achieve timeout, cancel read task", self->id_);
    ec = self->serialPort_.cancel(ec);
    if (ec) {
      MG_WARN("ModbusRtuMaster({})::wait: socket cancel error: {}", self->id_, ec.message());
    }
  });
}
void ModbusRtuMaster::StartReadTask() {
  MG_TRACE("ModbusRtuMaster({})::StartReadTask", id_);

  auto modbusBuffer = std::make_shared<modbus::ModbusBuffer>(frameType_);
  Weak weak = GetWeak();
  serialPort_.async_read_some(asio::buffer(modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize()),
                              [weak, modbusBuffer](asio::error_code ec, size_t size) {
                                Ptr self = weak.lock();
                                if (!self) {
                                  MG_CRIT("ModbusRtuMaster::read: actor was deleted")
                                  return;
                                }

                                std::lock_guard<std::mutex> lock(self->m_);

                                try {
                                  const auto tp = self->timer_.expiry() - std::chrono::steady_clock::now();
                                  const auto exp = tp.count() / std::chrono::microseconds::period::den;
                                  MG_DEBUG("ModbusRtuMaster({})::read: expiry {}", self->id_, exp);
                                  self->timer_.cancel();
                                } catch (const asio::system_error &e) {
                                  MG_ERROR("ModbusRtuMaster({})::read: timer cancel error: {}", self->id_, ec.message());
                                }

                                if (ec) {
                                  MG_ERROR("ModbusRtuMaster({})::read: error: {}", self->id_, ec.message());
                                  self->currentMessage_.reset();
                                }

                                const auto modbusMessage = self->MakeResponse(modbusBuffer, size);
                                if (modbusMessage) {
                                  self->exchange_->Send(modbusMessage->GetModbusMessageInfo().GetSourceId(),
                                                        modbusMessage);
                                }

                                self->QueueProcessUnsafe();
                              });
}

ModbusMessagePtr ModbusRtuMaster::MakeResponse(const ModbusBufferPtr &modbusBuffer, size_t size) {
  if (!currentMessage_) {
    return nullptr;
  }

  MG_DEBUG("ModbusRtuMaster({})::MakeResponse: receive: {} bytes", id_, size);
  if (!modbusBuffer->SetAduSize(size)) {
    MG_ERROR("ModbusRtuMaster({})::MakeResponse: invalid adu size", id_);
    return nullptr;
  }

  MG_TRACE("ModbusRtuMaster({})::MakeResponse: response: [{:X}]", id_, fmt::join(*modbusBuffer, " "))

  const auto currentMessage = currentMessage_->modbusMessage;
  currentMessage_.reset();

  const ModbusMessageInfo currentInfo = currentMessage->GetModbusMessageInfo();

  {
    const auto wrapper = modbus::MakeModbusBufferWrapper(*modbusBuffer);
    const auto result = wrapper->Check();
    if (result != modbus::CheckFrameResult::NoError) {
      MG_ERROR("ModbusRtuMaster({})::MakeResponse: check message failed: {}", id_, result)
      return nullptr;
    }

    // maybe need some check?
  }

  return ModbusMessage::Create(currentInfo, modbusBuffer);
}

}// namespace modbus_gateway
// namespace modbus_gateway
