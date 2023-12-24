#include <transport/modbus_tcp_server.h>

#include <common/logger.h>
#include <message/client_disconnect_message.h>


using namespace asio;

namespace modbus_gateway {

ModbusTcpServer::ModbusTcpServer(const exchange::ExchangePtr &exchange, const ContextPtr &context,
                                 const asio::ip::address &addr, ip::port_type port,
                                 const RouterPtr &router)
        : exchange_(exchange), acceptor_(*context, TcpEndpoint(addr, port)), router_(router) {
    assert(exchange_);
    assert(router_);
    MG_TRACE("ModbusTcpServer({})::Ctor: {}:{}", GetIdStr(), addr.to_string(), port);
}

void ModbusTcpServer::Receive(const exchange::MessagePtr &message) {
    MG_TRACE("ModbusTcpServer({})::Receive", GetIdStr());
    auto ptr = dynamic_cast< ClientDisconnectMessage * >( message.get());
    if (ptr) {
        MG_TRACE("ModbusTcpServer({})::Receive: ClientDisconnectMessage", GetIdStr());
        ClientDisconnect(ptr->GetClientId());
        return;
    }
    MG_TRACE("ModbusTcpServer({})::Receive: unsupported message", GetIdStr());
}

void ModbusTcpServer::Start() {
    assert(GetId().value());
    MG_INFO("ModbusTcpServer({})::Start", GetIdStr());
    AcceptTask();
}

void ModbusTcpServer::Stop() {
    MG_INFO("ModbusTcpServer({})::Stop", GetIdStr());
    error_code ec;
    acceptor_.close(ec);
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for (const auto &[clientId, client]: clientDb_) {
            MG_INFO("ModbusTcpServer({})::Stop: remove client {}", GetIdStr(), clientId);
            client->Stop();
            exchange_->Delete(clientId);
            clientDb_.erase(clientId);
        }
    }
}

ModbusTcpServer::~ModbusTcpServer() {
    Stop();
}

void ModbusTcpServer::AcceptTask() {
    MG_TRACE("ModbusTcpServer({})::AcceptTask", GetIdStr());
    TcpSocketPtr socket = std::make_unique<TcpSocketPtr::element_type>(acceptor_.get_executor());
    Weak weak = GetWeak();
    acceptor_.async_accept(*socket, [weak, socket = std::move(socket)](error_code ec) mutable {
        Ptr self = weak.lock();
        if (!self) {
            MG_CRIT("ModbusTcpServer::accept: actor was deleted");
            return;
        }

        if (ec) {
            MG_ERROR("ModbusTcpServer({})::accept: error: {}", self->GetIdStr(), ec.message());
            if (error::operation_aborted == ec) {
                MG_INFO("ModbusTcpServer({})::accept: canceled", self->GetIdStr());
                return;
            }
            MG_TRACE("ModbusTcpServer({})::accept: start accept task", self->GetIdStr());
            self->AcceptTask();
            return;
        }
        MG_INFO("ModbusTcpServer({})::accept: connect from {}:{}", self->GetIdStr(),
                socket->remote_endpoint().address().to_string(),
                socket->remote_endpoint().port())
        auto tcpClient = ModbusTcpConnection::Create(self->exchange_, self->GetId().value(), std::move(socket),
                                                     self->router_);
        exchange::ActorId clientId = 0;
        {
            std::unique_lock<std::mutex> lock(self->mutex_);
            clientId = self->exchange_->Add(tcpClient);
            self->clientDb_[clientId] = tcpClient;
        }
        MG_TRACE("ModbusTcpServer({})::accept: start clientId {}", self->GetIdStr(), clientId);
        tcpClient->Start();
        MG_TRACE("ModbusTcpServer({})::accept: start accept task", self->GetIdStr());
        self->AcceptTask();
    });
}

void ModbusTcpServer::ClientDisconnect(exchange::ActorId clientId) {
    std::unique_lock<std::mutex> lock(mutex_);
    MG_INFO("ModbusTcpServer({})::ClientDisconnect: remove client {}", GetIdStr(), clientId);
    exchange_->Delete(clientId);
    clientDb_.erase(clientId);
}

const std::string &ModbusTcpServer::GetIdStr() const {
    const auto id = GetId();
    if (id.has_value()) {
        static std::string idStr = std::to_string(id.value());
        return idStr;
    }

    static std::string empty;
    return empty;
}

}
