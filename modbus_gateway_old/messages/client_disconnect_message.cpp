#include "client_disconnect_message.h"

namespace modbus_gateway
{

ClientDisconnectMessage::ClientDisconnectMessage( exchange::ActorId clientId )
          : clientId_( clientId )
{}

exchange::ActorId ClientDisconnectMessage::GetClientId() const
{
     return clientId_;
}

}
