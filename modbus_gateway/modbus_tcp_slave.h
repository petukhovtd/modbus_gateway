#ifndef MODBUS_GATEWAY_MODBUS_TCP_SLAVE_H
#define MODBUS_GATEWAY_MODBUS_TCP_SLAVE_H

#include <exchange/actor_helper.h>
#include <messages/modbus_message.h>
#include <types.h>
#include <common/synchronized.h>

namespace modbus_gateway
{

class ModbusTcpSlave: public exchange::ActorHelper< ModbusTcpSlave >
{
     using ModbusMessageInfoOpt =  std::optional< ModbusMessageInfo >;

public:
     explicit ModbusTcpSlave( exchange::ActorId serverId, TcpSocketPtr socket, std::chrono::milliseconds requestTimeout, const RouterPtr& router );

     ~ModbusTcpSlave() override = default;

     void Receive( const exchange::MessagePtr& ) override;

     void Start();

     void Stop();

private:
     static ModbusMessagePtr MakeRequest( const ModbusBufferPtr& modbusBuffer, size_t size, exchange::ActorId masterId );

     void StartReadTask();

     void StartTimeoutTask();

     ModbusBufferPtr MakeResponse( const ModbusMessage& modbusMessage );

     void SyncWriteMessage( const ModbusMessage& modbusMessage );

private:
     exchange::ActorId serverId_;
     TcpSocketPtr socket_;
     const std::chrono::milliseconds requestTmeout_;
     WaitTimerPtr requestTimer_;
     RouterPtr router_;
     Synchronized< ModbusMessageInfoOpt > syncRequestInfo_;
};

}


#endif
