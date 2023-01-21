#ifndef MODBUS_GATEWAY_CLIENT_DISCONNECT_MESSAGE_H
#define MODBUS_GATEWAY_CLIENT_DISCONNECT_MESSAGE_H

#include <exchange/message_helper.h>
#include <exchange/id.h>

namespace modbus_gateway
{

class ClientDisconnectMessage
          : public exchange::MessageHelper< ClientDisconnectMessage >
{
public:
     explicit ClientDisconnectMessage( exchange::ActorId clientId );

     [[nodiscard]] exchange::ActorId GetClientId() const;

private:
     exchange::ActorId clientId_;
};

}

#endif
