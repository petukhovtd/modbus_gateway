#pragma once

#include <exchange/id.h>
#include <exchange/message_helper.h>

namespace modbus_gateway {

class ClientDisconnectMessage
    : public exchange::MessageHelper<ClientDisconnectMessage> {
public:
  explicit ClientDisconnectMessage(exchange::ActorId clientId);

  exchange::ActorId GetClientId() const;

private:
  exchange::ActorId clientId_;
};

}// namespace modbus_gateway
