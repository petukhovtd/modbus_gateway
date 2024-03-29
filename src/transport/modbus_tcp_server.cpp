#include <transport/modbus_tcp_server.h>

#include <common/logger.h>
#include <message/client_disconnect_message.h>

using namespace asio;

namespace modbus_gateway {

ModbusTcpServer::ModbusTcpServer(const exchange::ExchangePtr &exchange, const ContextPtr &context,
                                 const asio::ip::address &addr, ip::port_type port,
                                 const RouterPtr &router)
    : IModbusSlave(TransportType::TcpServer),
      id_(exchange::defaultId),
      exchange_(exchange),
      acceptor_(*context, TcpEndpoint(addr, port)),
      router_(router) {
  assert(router_);
  MG_DEBUG("ModbusTcpServer({})::Ctor: {}:{}", id_, addr.to_string(), port);
}

void ModbusTcpServer::Receive(const exchange::MessagePtr &message) {
  auto ptr = dynamic_cast<ClientDisconnectMessage *>(message.get());
  if (ptr) {
    MG_TRACE("ModbusTcpServer({})::Receive: ClientDisconnectMessage", id_);
    ClientDisconnect(ptr->GetClientId());
    return;
  }
  MG_WARN("ModbusTcpServer({})::Receive: unsupported message", id_);
}

void ModbusTcpServer::SetId(exchange::ActorId id) {
  id_ = id;
}

void ModbusTcpServer::ResetId() {
  id_ = exchange::defaultId;
}

exchange::ActorId ModbusTcpServer::GetId() {
  return id_;
}

void ModbusTcpServer::Start() {
  assert(id_ != exchange::defaultId);
  MG_DEBUG("ModbusTcpServer({})::Start", id_);
  AcceptTask();
}

void ModbusTcpServer::Stop() {
  MG_DEBUG("ModbusTcpServer({})::Stop", id_);
  error_code ec;
  ec = acceptor_.cancel(ec);
  if (ec) {
    MG_WARN("ModbusTcpServer({})::Stop acceptor cancel error: {}", id_, ec.message());
  }
  {
    std::scoped_lock<std::mutex> lock(mutex_);
    auto exchange = exchange_.lock();
    for (const auto &[clientId, client] : clientDb_) {
      MG_INFO("ModbusTcpServer({})::Stop: remove client {}", id_, clientId);
      client->Stop();
      if (exchange) {
        exchange->Delete(clientId);
      }
    }
    clientDb_.clear();
  }
}

ModbusTcpServer::~ModbusTcpServer() {
  MG_DEBUG("ModbusTcpServer({})::Dtor", id_);
  Stop();
  error_code ec;
  ec = acceptor_.close(ec);
  if (ec) {
    MG_WARN("ModbusTcpServer({})::Dtor acceptor close error: {}", id_, ec.message());
  }
}

void ModbusTcpServer::AcceptTask() {
  MG_TRACE("ModbusTcpServer({})::AcceptTask", id_);
  auto rawSocket = new TcpSocketPtr::element_type(acceptor_.get_executor());
  TcpSocketPtr socket = std::unique_ptr<TcpSocketPtr::element_type>(rawSocket);
  Weak weak = GetWeak();
  acceptor_.async_accept(*rawSocket, [weak, socket = std::move(socket)](error_code ec) mutable {
    Ptr self = weak.lock();
    if (!self) {
      MG_WARN("ModbusTcpServer::accept: actor was deleted");
      return;
    }
    auto exchange = self->exchange_.lock();
    if (!exchange) {
      MG_WARN("ModbusTcpServer({})::accept: exchange was deleted");
      return;
    }

    if (ec) {
      if (error::operation_aborted == ec) {
        MG_INFO("ModbusTcpServer({})::accept: canceled", self->id_);
        return;
      }
      MG_ERROR("ModbusTcpServer({})::accept: error: {}", self->id_, ec.message());
      self->AcceptTask();
      return;
    }
    MG_INFO("ModbusTcpServer({})::accept: connect from {}:{}", self->id_,
            socket->remote_endpoint().address().to_string(),
            socket->remote_endpoint().port())
    auto tcpClient = ModbusTcpConnection::Create(exchange, self->id_, std::move(socket),
                                                 self->router_);
    exchange::ActorId clientId = 0;
    {
      std::scoped_lock<std::mutex> lock(self->mutex_);
      clientId = exchange->Add(tcpClient);
      self->clientDb_[clientId] = tcpClient;
    }
    tcpClient->Start();
    self->AcceptTask();
  });
}

void ModbusTcpServer::ClientDisconnect(exchange::ActorId clientId) {
  std::scoped_lock<std::mutex> lock(mutex_);
  MG_INFO("ModbusTcpServer({})::ClientDisconnect: remove client {}", id_, clientId);
  auto exchange = exchange_.lock();
  if (exchange) {
    exchange->Delete(clientId);
  }
  clientDb_.erase(clientId);
}

}// namespace modbus_gateway
