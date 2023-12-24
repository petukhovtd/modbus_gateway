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
        : exchange_(exchange), serverId_(serverId), socket_(std::move(socket)), router_(router),
          syncRequestInfo_(std::nullopt) {
    assert(exchange_);
    assert(socket_);
    assert(router_);
    MG_TRACE("ModbusTcpConnection()::Ctor: serverId {}", serverId_);
}

void ModbusTcpConnection::Receive(const exchange::MessagePtr &message) {
    MG_TRACE("ModbusTcpConnection({})::Receive", GetIdStr());
    const ModbusMessagePtr &modbusMessage = std::dynamic_pointer_cast<ModbusMessagePtr::element_type>(message);
    if (modbusMessage) {
        MG_TRACE("ModbusTcpConnection({})::Receive: ModbusMessage", GetIdStr());
        StartSendTask(modbusMessage);
        return;
    }
    MG_TRACE("ModbusTcpConnection({})::Receive: unsupported message", GetIdStr());
}

void ModbusTcpConnection::Start() {
    assert(GetId().has_value());
    MG_INFO("ModbusTcpConnection({})::Start: serverId {}, client {}:{}",
            GetIdStr(), serverId_,
            socket_->remote_endpoint().address().to_string(),
            socket_->remote_endpoint().port())
    StartReceiveTask();
}

void ModbusTcpConnection::Stop() {
    error_code ec;
    socket_->close(ec);
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
    MG_TRACE("ModbusTcpConnection({})::StartReadTask", GetIdStr());
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
                                   MG_ERROR("ModbusTcpConnection({})::receive: error: {}", self->GetIdStr(),
                                            ec.message())
                                   if ((error::eof == ec) || (error::connection_reset == ec) ||
                                       (error::operation_aborted == ec)) {
                                       MG_INFO("ModbusTcpConnection({})::receive: send disconnect message",
                                               self->GetIdStr());
                                       self->exchange_->Send(self->serverId_,
                                                             ClientDisconnectMessage::Create(
                                                                     self->GetId().value()));
                                       return;
                                   }
                                   MG_TRACE("ModbusTcpConnection({})::receive: start receive task",
                                            self->GetIdStr());
                                   self->StartReceiveTask();
                                   return;
                               }

                               MG_DEBUG("ModbusTcpConnection({})::receive: {} bytes", self->GetIdStr(), size);
                               auto message = self->MakeRequest(modbusBuffer, size, self->GetId().value());
                               if (!message) {
                                   MG_ERROR("ModbusTcpConnection: receive: invalid request, start receive task");
                                   self->StartReceiveTask();
                                   return;
                               }

                               {
                                   MG_TRACE("ModbusTcpConnection({})::receive: update modbus message info",
                                            self->GetIdStr());
                                   auto access = self->syncRequestInfo_.GetAccess();
                                   access.ref = message->GetModbusMessageInfo();
                               }

                               const modbus::UnitId unitId = message->GetModbusBuffer()->GetUnitId();
                               const exchange::ActorId slaveId = self->router_->Route(unitId);
                               MG_TRACE("ModbusTcpConnection({})::receive: unit id {} route to slave id {}",
                                        self->GetIdStr(), message->GetModbusMessageInfo().GetSourceId(), slaveId);
                               const auto res = self->exchange_->Send(slaveId, message);
                               if (!res) {
                                   MG_ERROR("ModbusTcpConnection({})::receive: route to slave id {} failed",
                                            self->GetIdStr(), slaveId);
                               }
                               MG_TRACE("ModbusTcpConnection({})::receive: start receive task", self->GetIdStr());
                               self->StartReceiveTask();
                           });
}

ModbusBufferPtr ModbusTcpConnection::MakeResponse(const ModbusMessagePtr &modbusMessage) {
    MG_TRACE("ModbusTcpConnection({})::MakeResponse", GetIdStr());

    const ModbusMessageInfo &messageInfo = modbusMessage->GetModbusMessageInfo();
    ModbusBufferPtr modbusBuffer = modbusMessage->GetModbusBuffer();

    MG_TRACE("ModbusTcpConnection({})::MakeResponse: message source id {}, transaction id {}", GetIdStr(),
             messageInfo.GetSourceId(), messageInfo.GetTransactionId());

    if (GetId().value() != messageInfo.GetSourceId()) {
        MG_CRIT("ModbusTcpConnection({})::MakeResponse: invalid message source id {}",
                GetIdStr(),
                messageInfo.GetSourceId());
        return nullptr;
    }

    {
        auto access = syncRequestInfo_.GetAccess();
        ModbusMessageInfoOpt &modbusMessageInfoOpt = access.ref;
        if (!modbusMessageInfoOpt.has_value()) {
            MG_ERROR(
                    "ModbusTcpConnection({})::MakeResponse: last message info is empty, message transaction id {}",
                    GetIdStr(),
                    messageInfo.GetTransactionId());
            return nullptr;
        }
        const ModbusMessageInfo &lastInfo = modbusMessageInfoOpt.value();
        if (lastInfo.GetTransactionId() != messageInfo.GetTransactionId()) {
            MG_ERROR(
                    "ModbusTcpConnection({})::MakeResponse: message transaction id {} not equal last transaction id {}",
                    GetIdStr(),
                    messageInfo.GetTransactionId(), lastInfo.GetTransactionId());
            return nullptr;
        }
        MG_TRACE("ModbusTcpConnection({})::MakeResponse: message info checking successfully", GetIdStr());
        modbusMessageInfoOpt.reset();
    }

    if (!modbusBuffer) {
        MG_CRIT("ModbusTcpConnection({})::MakeResponse: modbus buffer is null. transaction id {}", GetIdStr(),
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
             GetIdStr(),
             modbusBufferTcpWrapper.GetTransactionId(),
             modbusBufferTcpWrapper.GetProtocolId(),
             modbusBufferTcpWrapper.GetLength(),
             modbusBuffer->GetUnitId(),
             modbusBuffer->GetFunctionCode());

    MG_TRACE("ModbusTcpConnection({})::MakeResponse: response: [{:X}]", GetIdStr(), fmt::join(*modbusBuffer, " "));

    return modbusBuffer;
}

void ModbusTcpConnection::StartSendTask(const ModbusMessagePtr &modbusMessage) {
    MG_TRACE("ModbusTcpConnection({})::StartSendTask", GetIdStr());
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
                                MG_ERROR("ModbusTcpConnection({})::send: error: {}", self->GetIdStr(), ec.message());
                                return;
                            }

                            MG_DEBUG("ModbusTcpConnection({})::send: {} bytes", self->GetIdStr(), size);
                        });
}

const std::string &ModbusTcpConnection::GetIdStr() const {
    const auto id = GetId();
    if (id.has_value()) {
        static std::string idStr = std::to_string(id.value());
        return idStr;
    }

    static std::string empty;
    return empty;
}

}
