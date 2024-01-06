#include <transport/modbus_tcp_server.h>

#include <common/logger.h>
#include <message/client_disconnect_message.h>

using namespace asio;

namespace modbus_gateway {

ModbusTcpServer::ModbusTcpServer(const exchange::ExchangePtr &exchange, const ContextPtr &context,
                                 const asio::ip::address &addr, ip::port_type port,
                                 const RouterPtr &router)
    : id_(exchange::startId), exchange_(exchange), acceptor_(*context, TcpEndpoint(addr, port), true), router_(router) {
  assert(exchange_);
  assert(router_);
  MG_TRACE("ModbusTcpServer({})::Ctor: {}:{}", id_, addr.to_string(), port);
}

void ModbusTcpServer::Receive(const exchange::MessagePtr &message) {
  MG_TRACE("ModbusTcpServer({})::Receive", id_);
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
  id_ = exchange::startId;
}

void ModbusTcpServer::Start() {
  assert(id_ != exchange::startId);
  MG_INFO("ModbusTcpServer({})::Start", id_);
  AcceptTask();
}

void ModbusTcpServer::Stop() {
  MG_INFO("ModbusTcpServer({})::Stop", id_);
  error_code ec;
  ec = acceptor_.cancel(ec);
  if (ec) {
    MG_WARN("ModbusTcpServer({})::Stop acceptor cancel error: {}", id_, ec.message());
  }
  {
    std::scoped_lock<std::mutex> lock(mutex_);
    for (const auto &[clientId, client] : clientDb_) {
      MG_INFO("ModbusTcpServer({})::Stop: remove client {}", id_, clientId);
      client->Stop();
      exchange_->Delete(clientId);
    }
    clientDb_.clear();
  }
}

ModbusTcpServer::~ModbusTcpServer() {
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
      MG_CRIT("ModbusTcpServer::accept: actor was deleted");
      return;
    }

    if (ec) {
      MG_ERROR("ModbusTcpServer({})::accept: error: {}", self->id_, ec.message());
      if (error::operation_aborted == ec) {
        MG_INFO("ModbusTcpServer({})::accept: canceled", self->id_);
        return;
      }
      MG_TRACE("ModbusTcpServer({})::accept: start accept task", self->id_);
      self->AcceptTask();
      return;
    }
    MG_INFO("ModbusTcpServer({})::accept: connect from {}:{}", self->id_,
            socket->remote_endpoint().address().to_string(),
            socket->remote_endpoint().port())
    auto tcpClient = ModbusTcpConnection::Create(self->exchange_, self->id_, std::move(socket),
                                                 self->router_);
    exchange::ActorId clientId = 0;
    {
      std::scoped_lock<std::mutex> lock(self->mutex_);
      clientId = self->exchange_->Add(tcpClient);
      self->clientDb_[clientId] = tcpClient;
    }
    MG_TRACE("ModbusTcpServer({})::accept: start clientId {}", self->id_, clientId);
    tcpClient->Start();
    MG_TRACE("ModbusTcpServer({})::accept: start accept task", self->id_);
    self->AcceptTask();
  });
}

void ModbusTcpServer::ClientDisconnect(exchange::ActorId clientId) {
  std::scoped_lock<std::mutex> lock(mutex_);
  MG_INFO("ModbusTcpServer({})::ClientDisconnect: remove client {}", id_, clientId);
  exchange_->Delete(clientId);
  clientDb_.erase(clientId);
}

}// namespace modbus_gateway
