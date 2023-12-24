#pragma once

#include <exchange/message_helper.h>
#include <exchange/id.h>

namespace modbus_gateway {

class ClientDisconnectMessage
        : public exchange::MessageHelper<ClientDisconnectMessage> {
public:
    explicit ClientDisconnectMessage(exchange::ActorId clientId);

    exchange::ActorId GetClientId() const;

private:
    exchange::ActorId clientId_;
};

}
