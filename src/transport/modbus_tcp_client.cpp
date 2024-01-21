#include <transport/modbus_tcp_client.h>

#include <common/logger.h>
#include <modbus/modbus_buffer_tcp_wrapper.h>

namespace modbus_gateway {

ModbusTcpClient::ModbusTcpClient(const exchange::ExchangePtr &exchange,
                                 const ContextPtr &context,
                                 const asio::ip::address &addr,
                                 asio::ip::port_type port,
                                 std::chrono::milliseconds timeout)
    : id_(exchange::defaultId),
      exchange_(exchange),
      socket_(std::make_unique<TcpSocketPtr::element_type>(*context)),
      ep_(addr, port),
      timeout_(timeout),
      timer_(socket_->get_executor()),
      m_(), messageQueue_(),
      currentMessage_(std::nullopt),
      transactionIdGenerator_(0),
      state_(State::Idle) {
  MG_DEBUG("ModbusTcpClient({})::Ctor: {}:{}", id_, addr.to_string(), port);
}

ModbusTcpClient::~ModbusTcpClient() {
  MG_DEBUG("ModbusTcpClient({})::Dtor: {}:{}", id_, ep_.address().to_string(), ep_.port());
  CloseSocket();
}

void ModbusTcpClient::Receive(const exchange::MessagePtr &message) {
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
  id_ = exchange::defaultId;
}

exchange::ActorId ModbusTcpClient::GetId() {
  return id_;
}

void ModbusTcpClient::MessageProcess(const ModbusMessagePtr &message) {
  std::lock_guard<std::mutex> lock(m_);
  messageQueue_.Push(message);
  MG_TRACE("ModbusTcpClient({})::MessageProcess: message in queue {}", id_, messageQueue_.Size());
  QueueProcessUnsafe();
}

void ModbusTcpClient::QueueProcessUnsafe() {
  if (messageQueue_.Empty()) {
    MG_TRACE("ModbusTcpClient({})::QueueProcess: queue is empty", id_);
    return;
  }

  MG_DEBUG("ModbusTcpClient({})::QueueProcess: queue size: {}, state: {}", id_, messageQueue_.Size(), StateToStr(state_));

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
  MG_INFO("ModbusTcpClient({})::StartConnectTask: connect to {}:{}", id_, ep_.address().to_string(), ep_.port());
  Weak weak = GetWeak();
  socket_->async_connect(ep_, [weak](asio::error_code ec) {
    Ptr self = weak.lock();
    if (!self) {
      MG_WARN("ModbusTcpClient::connect: actor was deleted")
      return;
    }

    std::lock_guard<std::mutex> lock(self->m_);

    if (ec) {
      self->state_ = State::Idle;

      if ((asio::error::operation_aborted == ec)) {
        MG_INFO("ModbusTcpClient({})::connect: canceled", self->id_);
        return;
      }
      MG_ERROR("ModbusTcpClient({})::connect: error: {}", self->id_, ec.message());
      return;
    }

    self->state_ = State::Connected;

    MG_INFO("ModbusTcpClient({})::connect: connect to {}:{} successful", self->id_,
            self->socket_->remote_endpoint().address().to_string(),
            self->socket_->remote_endpoint().port());
    self->QueueProcessUnsafe();
  });
}

void ModbusTcpClient::StartMessageTaskUnsafe() {
  if (currentMessage_) {
    MG_DEBUG("ModbusTcpClient({})::StartMessageTask: message in process", id_);
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
  const auto originType = modbusBuffer->GetType();
  modbusBuffer->ConvertTo(modbus::FrameType::TCP);
  {
    modbus::ModbusBufferTcpWrapper modbusBufferTcpWrapper(*modbusBuffer);
    modbusBufferTcpWrapper.Update();
    modbusBufferTcpWrapper.SetTransactionId(currentMessage_->id);

    MG_DEBUG("ModbusTcpClient({})::StartMessageTask: frame type {}->{}, actor id {}, transaction Id {}->{}",
             id_,
             static_cast<int>(originType),
             static_cast<int>(modbus::FrameType::TCP),
             static_cast<int>(currentMessage_->modbusMessage->GetModbusMessageInfo().GetSourceId()),
             static_cast<int>(currentMessage_->modbusMessage->GetModbusMessageInfo().GetTransactionId()),
             static_cast<int>(currentMessage_->id));

    MG_DEBUG("ModbusTcpClient({})::StartMessageTask: request: [{:X}]", id_, fmt::join(*modbusBuffer, " "));
  }

  Weak weak = GetWeak();
  socket_->async_send(asio::buffer(modbusBuffer->begin().base(), modbusBuffer->GetAduSize()),
                      [weak](asio::error_code ec, size_t size) {
                        Ptr self = weak.lock();
                        if (!self) {
                          MG_WARN("ModbusTcpClient::send: actor was deleted")
                          return;
                        }

                        std::lock_guard<std::mutex> lock(self->m_);

                        if (ec) {
                          self->currentMessage_.reset();
                          if (asio::error::operation_aborted != ec) {
                            MG_INFO("ModbusTcpClient({})::send: close connection", self->id_);
                            self->state_ = State::Idle;
                            self->CloseSocket();
                          }
                          MG_ERROR("ModbusTcpClient({})::send: error {}", self->id_, ec.message());
                          return;
                        }

                        MG_TRACE("ModbusTcpClient({})::send: send {} bytes", self->id_, size);
                        self->StartWaitTask();
                        self->StartReceiveTask();
                      });
}

void ModbusTcpClient::StartWaitTask() {
  MG_TRACE("ModbusTcpClient({})::StartWaitTask timeout {}ms", id_, timeout_.count());
  timer_.expires_after(timeout_);

  Weak weak = GetWeak();
  timer_.async_wait([weak](asio::error_code ec) {
    Ptr self = weak.lock();
    if (!self) {
      MG_WARN("ModbusTcpClient::wait: actor was deleted");
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

    MG_ERROR("ModbusTcpClient({})::wait: achieve timeout, cancel receive task", self->id_);
    ec = self->socket_->cancel(ec);
    if (ec) {
      MG_WARN("ModbusTcpClient({})::wait: socket cancel error: {}", self->id_, ec.message());
    }
  });
}

ModbusMessagePtr ModbusTcpClient::MakeResponse(const ModbusBufferPtr &modbusBuffer, size_t size) {
  if (!currentMessage_) {
    MG_ERROR("ModbusTcpClient({})::MakeResponse: current message is empty", id_);
    return nullptr;
  }

  if (!modbusBuffer->SetAduSize(size)) {
    MG_ERROR("ModbusTcpClient({})::MakeResponse: invalid adu size {}", id_, size);
    return nullptr;
  }

  MG_DEBUG("ModbusTcpClient({})::MakeResponse: response: [{:X}]", id_, fmt::join(*modbusBuffer, " "));

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
  MG_DEBUG("ModbusTcpClient({})::MakeResponse: response: actor id {}, transaction id {}->{}",
           id_,
           currentInfo.GetSourceId(),
           id,
           currentInfo.GetTransactionId());
  return ModbusMessage::Create(currentInfo, modbusBuffer);
}

void ModbusTcpClient::StartReceiveTask() {
  MG_TRACE("ModbusTcpClient({})::StartReceiveTask", id_);

  auto modbusBuffer = std::make_shared<modbus::ModbusBuffer>(modbus::FrameType::TCP);
  Weak weak = GetWeak();
  socket_->async_receive(asio::buffer(modbusBuffer->begin().operator->(), modbusBuffer->GetAduSize()),
                         [weak, modbusBuffer](asio::error_code ec, size_t size) {
                           Ptr self = weak.lock();
                           if (!self) {
                             MG_WARN("ModbusTcpClient::receive: actor was deleted")
                             return;
                           }

                           auto exchange = self->exchange_.lock();
                           if (!exchange) {
                             MG_WARN("ModbusTcpClient({})::accept: exchange was deleted");
                             return;
                           }

                           std::lock_guard<std::mutex> lock(self->m_);

                           try {
                             const auto tp = self->timer_.expiry() - std::chrono::steady_clock::now();
                             const auto exp = tp.count() / std::chrono::microseconds::period::den;
                             MG_DEBUG("ModbusTcpClient({})::receive: left {}ms", self->id_, exp);
                             self->timer_.cancel();
                           } catch (const asio::system_error &e) {
                             MG_ERROR("ModbusTcpClient({})::receive: timer cancel error: {}", self->id_, ec.message());
                           }

                           if (ec) {
                             self->currentMessage_.reset();
                             if (asio::error::operation_aborted != ec) {
                               MG_INFO("ModbusTcpClient({})::receive: close connection", self->id_);
                               self->state_ = State::Idle;
                               self->CloseSocket();
                               return;
                             }
                             MG_ERROR("ModbusTcpClient({})::receive: error: {}", self->id_, ec.message());
                             return;
                           }

                           MG_TRACE("ModbusTcpClient({})::receive: receive {} bytes", self->id_, size);

                           const auto modbusMessage = self->MakeResponse(modbusBuffer, size);
                           if (modbusMessage) {
                             const auto actorId = modbusMessage->GetModbusMessageInfo().GetSourceId();
                             const auto res = exchange->Send(actorId, modbusMessage);
                             if(!res){
                               MG_ERROR("ModbusTcpClient({})::receive: send to actorId {} failed", self->id_, actorId);
                             }
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
  MG_TRACE("ModbusTcpClient({})::CloseSocket", id_);
  if (socket_->is_open()) {
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
}

}// namespace modbus_gateway
