#ifndef MODBUS_GATEWAY_MODBUS_TCP_SLAVE_H
#define MODBUS_GATEWAY_MODBUS_TCP_SLAVE_H

#include <exchange/actor_helper.h>

#include <types.h>

namespace modbus_gateway
{

class ModbusTcpSlave
          : public exchange::ActorHelper< ModbusTcpSlave >
{
public:
     explicit ModbusTcpSlave( SocketPtr socket, exchange::ActorId serverId );

     void Receive( const exchange::MessagePtr& ) override;

     void Start();

     void Stop();

     ~ModbusTcpSlave() override;

private:
     void PushReadTask();

     void PushWriteTask( const DataPtr& data );

private:
     SocketPtr socket_;
     exchange::ActorId serverId_;
};

}


#endif
