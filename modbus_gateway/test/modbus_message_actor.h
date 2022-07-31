#ifndef MODBUS_GATEWAY_MODBUS_MESSAGE_ACTOR_H
#define MODBUS_GATEWAY_MODBUS_MESSAGE_ACTOR_H

#include <exchange/actor_helper.h>

#include <messages/modbus_message.h>

namespace test
{

class ModbusMessageActor: public exchange::ActorHelper< ModbusMessageActor >
{
public:
     using Handler = std::function< modbus_gateway::ModbusMessagePtr( const modbus_gateway::ModbusMessagePtr& ) >;

     explicit ModbusMessageActor( Handler handler );

     ~ModbusMessageActor() override = default;

     void Receive( const exchange::MessagePtr& message ) override;

private:
     Handler handler_;
};

}

#endif // MODBUS_GATEWAY_MODBUS_MESSAGE_ACTOR_H
