#include <transport/modbus_tcp_client.h>

#include <common/logger.h>
#include <modbus/modbus_buffer_tcp_wrapper.h>

//#include <exchange/exchange.h>

namespace modbus_gateway {

ModbusTcpClient::ModbusTcpClient(const exchange::ExchangePtr &exchange,
                                 const ContextPtr &context,
                                 const asio::ip::address &addr,
                                 asio::ip::port_type port,
                                 std::chrono::milliseconds timeout)
    : id_(exchange::startId),
      exchange_(exchange),
      socket_(std::make_unique<TcpSocketPtr::element_type>(*context)),
      ep_(addr, port),
      timeout_(timeout),
      timer_(socket_->get_executor()),
      m_(), messageQueue_(),
      currentMessage_(std::nullopt),
      transactionIdGenerator_(0),
      state_(State::Idle) {
  MG_TRACE("ModbusTcpClient({})::Ctor: {}:{}", id_, addr.to_string(), port);
}

ModbusTcpClient::~ModbusTcpClient() {
  MG_TRACE("ModbusTcpClient({})::Dtor: {}:{}", id_);
  if (socket_->is_open()) {
    CloseSocket();
  }
}

void ModbusTcpClient::Receive(const exchange::MessagePtr &message) {
  MG_TRACE("ModbusTcpClient({})::Receive", id_);
  auto modbusMessage = std::dynamic_pointer_cast<ModbusMessagePtr::element_type>(message);
  if (modbusMessage) {
    MG_TRACE("ModbusTcpClient({})::Receive: ModbusMessage", id_);
    MessageProcess(modbusMessage);
    return;
  }
  MG_WARN("ModbusTcpClient({})::Receive: unsupported message", id_);
}

void ModbusTcpClient::SetId(exchange::ActorId id) {
  id_ = id;
}

void ModbusTcpClient::ResetId() {
  id_ = exchange::startId;
}

void ModbusTcpClient::MessageProcess(const ModbusMessagePtr &message) {
  std::lock_guard<std::mutex> lock(m_);
  messageQueue_.Push(message);
  MG_TRACE("ModbusTcpClient({})::MessageProcess: message in queue {}", id_, messageQueue_.Size());

  QueueProcessUnsafe();
}

void ModbusTcpClient::QueueProcessUnsafe() {
  MG_TRACE("ModbusTcpClient({})::QueueProcess", id_);
  if (messageQueue_.Empty()) {
    MG_DEBUG("ModbusTcpClient({})::QueueProcess: queue is empty", id_);
    return;
  }

  MG_TRACE("ModbusTcpClient({})::QueueProcess: state: {}", id_, StateToStr(state_));

  switch (state_) {
  case State::Idle:
    StartConnectTaskUnsafe();
    break;
  case State::WaitConnect:
    MG_DEBUG("ModbusTcpClient({})::QueueProcess: wait connect", id_);
    break;
  case State::Connected:
    StartMessageTaskUnsafe();
    break;
  case State::MessageProcess:
    MG_DEBUG("ModbusTcpClient({})::QueueProcess: message is in the process", id_);
    break;
  }
}

void ModbusTcpClient::StartConnectTaskUnsafe() {
  MG_INFO("ModbusTcpClient({})::StartConnectTask: connect to {}:{}", id_, ep_.address().to_string(), ep_.port())
  Weak weak = GetWeak();
  socket_->async_connect(ep_, [weak](asio::error_code ec) {
    Ptr self = weak.lock();
    if (!self) {
      MG_CRIT("ModbusTcpClient::connect: actor was deleted")
      return;
    }

    std::lock_guard<std::mutex> lock(self->m_);

    if (ec) {
      self->state_ = State::Idle;

      MG_TRACE("ModbusTcpClient({})::connect: error: {}", self->id_, ec.message());
      if ((asio::error::operation_aborted == ec)) {
        MG_INFO("ModbusTcpClient({})::connect: canceled", self->id_);
        return;
      }
      MG_ERROR("ModbusTcpClient({})::connect: error: {}", self->id_, ec.message());
      // Что бы не сыпать ошибки в лог при отстуствии сервера, подключаться будем только при новом сообщении
      //                    MG_INFO( "ModbusTcpMaster: connect: start connect task" )
      //                    self->StartConnectTask();
      return;
    }

    self->state_ = State::Connected;

    MG_INFO("ModbusTcpClient({})::connect: connect to {}:{} successful", self->id_,
            self->socket_->remote_endpoint().address().to_string(),
            self->socket_->remote_endpoint().port())

    MG_TRACE("ModbusTcpClient({})::connect: start queue process", self->id_);
    self->QueueProcessUnsafe();
  });
}

void ModbusTcpClient::StartMessageTaskUnsafe() {
  MG_TRACE("ModbusTcpClient({})::StartMessageTask", id_);
  if (currentMessage_) {
    MG_INFO("ModbusTcpClient({})::StartMessageTask: message in process", id_);
    return;
  }

  while (!messageQueue_.Empty()) {
    const auto &message = messageQueue_.Front();
    if (!message->GetModbusMessageInfo().TimeoutReached(timeout_)) {
      break;
    }

    const auto &messageInfo = message->GetModbusMessageInfo();
    MG_INFO(
        "ModbusTcpClient({})::StartMessageTask: message reached timeout {}, message id {}, source id {}", id_,
        timeout_.count(), messageInfo.GetTransactionId(), messageInfo.GetSourceId());
    messageQueue_.Pop();
  }

  if (messageQueue_.Empty()) {
    MG_INFO("ModbusTcpClient({})::StartMessageTask: message queue empty", id_);
    return;
  }

  state_ = State::MessageProcess;
  currentMessage_ = {messageQueue_.Front(), ++transactionIdGenerator_};
  messageQueue_.Pop();

  ModbusBufferPtr modbusBuffer = currentMessage_->modbusMessage->GetModbusBuffer();
  modbusBuffer->ConvertTo(modbus::FrameType::TCP);
  {
    modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper(*modbusBuffer);
    modbusBufferTcpWrapper.Update();
    const auto originTransactionId = modbusBufferTcpWrapper.GetTransactionId();
    modbusBufferTcpWrapper.SetTransactionId(currentMessage_->id);

    MG_DEBUG("ModbusTcpClient({})::StartMessageTask: request: transaction id {}, "
             "origin transaction id {}, "
             "protocol id {}, "
             "length {}, "
             "unit id {}, "
             "function code {}",
             id_,
             modbusBufferTcpWrapper.GetTransactionId(),
             originTransactionId,
             modbusBufferTcpWrapper.GetProtocolId(),
             modbusBufferTcpWrapper.GetLength(),
             modbusBuffer->GetUnitId(),
             modbusBuffer->GetFunctionCode())

    MG_DEBUG("ModbusTcpClient({})::StartMessageTask: request: [{:X}]", id_, fmt::join(*modbusBuffer, " "));
  }

  Weak weak = GetWeak();
  socket_->async_send(asio::buffer(modbusBuffer->begin().base(), modbusBuffer->GetAduSize()),
                      [weak](asio::error_code ec, size_t size) {
                        Ptr self = weak.lock();
                        if (!self) {
                          MG_CRIT("ModbusTcpClient::send: actor was deleted")
                          return;
                        }

                        std::lock_guard<std::mutex> lock(self->m_);

                        if (ec) {
                          MG_ERROR("ModbusTcpClient({})::send: error {}", self->id_, ec.message());
                          self->currentMessage_.reset();
                          if (asio::error::operation_aborted != ec) {
                            MG_INFO("ModbusTcpClient({})::send: close connection", self->id_);
                            self->state_ = State::Idle;
                            self->CloseSocket();
                          }
                          return;
                        }

                        MG_TRACE("ModbusTcpClient({})::send: start wait task", self->id_);
                        self->StartWaitTask();
                        MG_TRACE("ModbusTcpClient({})::send: start receive task", self->id_);
                        self->StartReceiveTask();
                      });
}

void ModbusTcpClient::StartWaitTask() {
  MG_TRACE("ModbusTcpClient({})::StartWaitTask", id_);

  MG_DEBUG("ModbusTcpClient({})::StartWaitTask timeout {}ms", id_, timeout_.count())
  timer_.expires_after(timeout_);

  Weak weak = GetWeak();
  timer_.async_wait([weak](asio::error_code ec) {
    Ptr self = weak.lock();
    if (!self) {
      MG_CRIT("ModbusTcpClient::wait: actor was deleted")
      return;
    }

    std::lock_guard<std::mutex> lock(self->m_);

    if (ec) {
      if (asio::error::operation_aborted == ec) {
        MG_TRACE("ModbusTcpClient({})::wait: canceled", self->id_);
        return;
      }
      MG_ERROR("ModbusTcpClient({})::wait: error: {}", self->id_, ec.message())
      return;
    }

    MG_TRACE("ModbusTcpClient({})::wait: achieve timeout, cancel receive task", self->id_);
    ec = self->socket_->cancel(ec);
    if (ec) {
      MG_WARN("ModbusTcpClient({})::wait: socket cancel error: {}", self->id_, ec.message());
    }
  });
}

ModbusMessagePtr ModbusTcpClient::MakeResponse(const ModbusBufferPtr &modbusBuffer, size_t size) {
  if (!currentMessage_) {
    return nullptr;
  }

  MG_DEBUG("ModbusTcpClient({})::MakeResponse: receive: {} bytes", id_, size);
  if (!modbusBuffer->SetAduSize(size)) {
    MG_ERROR("ModbusTcpClient({})::MakeResponse: invalid adu size", id_);
    return nullptr;
  }

  MG_TRACE("ModbusTcpClient({})::MakeResponse: response: [{:X}]", id_, fmt::join(*modbusBuffer, " "))

  const auto currentMessage = currentMessage_->modbusMessage;
  const auto id = currentMessage_->id;
  currentMessage_.reset();

  const ModbusMessageInfo currentInfo = currentMessage->GetModbusMessageInfo();

  {
    modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper(*modbusBuffer);
    const auto result = modbusBufferTcpWrapper.Check();
    if (result != modbus::CheckFrameResult::NoError) {
      MG_ERROR("ModbusTcpClient({})::MakeResponse: check message failed: {}", id_, result)
      return nullptr;
    }

    if (modbusBufferTcpWrapper.GetTransactionId() != id) {
      MG_ERROR(
          "ModbusTcpClient({})::MakeResponse: receive message id {} not equal send message id {}", id_,
          modbusBufferTcpWrapper.GetTransactionId(), id)
      return nullptr;
    }
  }

  return ModbusMessage::Create(currentInfo, modbusBuffer);
}

void ModbusTcpClient::StartReceiveTask() {
  MG_TRACE("ModbusTcpClient({})::StartReceiveTask", id_);
  // Теортерически возможно переиспользовать и входной буффер, но необходимо его
  // отчистить и выставить максимальный размер
  auto modbusBuffer = std::make_shared<modbus::ModbusBuffer>(modbus::FrameType::TCP);
  Weak weak = GetWeak();
  socket_->async_receive(asio::buffer(modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize()),
                         [weak, modbusBuffer](asio::error_code ec, size_t size) {
                           Ptr self = weak.lock();
                           if (!self) {
                             MG_CRIT("ModbusTcpClient::receive: actor was deleted")
                             return;
                           }

                           std::lock_guard<std::mutex> lock(self->m_);

                           try {
                             const auto tp = self->timer_.expiry() - std::chrono::steady_clock::now();
                             const auto exp = tp.count() / std::chrono::microseconds::period::den;
                             MG_DEBUG("ModbusTcpClient({})::receive: expiry {}", self->id_, exp);
                             self->timer_.cancel();
                           } catch (const asio::system_error &e) {
                             MG_ERROR("ModbusTcpClient({})::receive: timer cancel error: {}", self->id_, ec.message());
                           }

                           if (ec) {
                             MG_ERROR("ModbusTcpClient({})::receive: error: {}", self->id_, ec.message());
                             self->currentMessage_.reset();
                             if (asio::error::operation_aborted != ec) {
                               MG_INFO("ModbusTcpClient({})::receive: close connection", self->id_);
                               self->state_ = State::Idle;
                               self->CloseSocket();
                               return;
                             }
                           }

                           const auto modbusMessage = self->MakeResponse(modbusBuffer, size);
                           if (modbusMessage) {
                             self->exchange_->Send(modbusMessage->GetModbusMessageInfo().GetSourceId(),
                                                   modbusMessage);
                           }

                           self->state_ = State::Connected;
                           self->QueueProcessUnsafe();
                         });
}

std::string ModbusTcpClient::StateToStr(State state) {
  switch (state) {
  case State::Idle: return "Idle";
  case State::WaitConnect: return "WaitConnect";
  case State::Connected: return "Connected";
  case State::MessageProcess: return "MessageProcess";
  default: return "Unknown";
  }
}

void ModbusTcpClient::CloseSocket() {
  asio::error_code ec;
  ec = socket_->shutdown(asio::socket_base::shutdown_both, ec);
  if (ec) {
    MG_WARN("ModbusTcpClient({})::CloseSocket: socket shutdown error: {}", id_, ec.message());
  }
  ec = socket_->close(ec);
  if (ec) {
    MG_WARN("ModbusTcpClient({})::CloseSocket: socket close error: {}", id_, ec.message());
  }
}

}// namespace modbus_gateway
