#ifndef MODBUS_GATEWAY_MODBUS_TCP_SLAVE_H
#define MODBUS_GATEWAY_MODBUS_TCP_SLAVE_H

#include <exchange/actor_helper.h>
#include <messages/modbus_message.h>
#include <types.h>

namespace modbus_gateway
{

class ModbusTcpSlave: public exchange::ActorHelper< ModbusTcpSlave >
{
public:
     explicit ModbusTcpSlave( exchange::ActorId serverId, SocketPtr socket, unsigned int timeoutMs, const RouterPtr& router );

     void Receive( const exchange::MessagePtr& ) override;

     void Start();

     void Stop();

     ~ModbusTcpSlave() override;

private:
     static ModbusMessagePtr MakeRequest( const ModbusBufferPtr& modbusBuffer, size_t size, exchange::ActorId masterId );

     void StartReadTask();

     void StartTimeoutTask();

     ModbusBufferPtr MakeResponse( const ModbusMessage& modbusMessage );

     void SyncWriteMessage( const ModbusMessage& modbusMessage );

private:
     exchange::ActorId serverId_;
     SocketPtr socket_;
     WaitTimerPtr waitTimer_;
     RouterPtr router_;

     std::mutex infoMutex_;
     std::optional< ModbusMessageInfo > lastRequestInfo_;
};

}


#endif
