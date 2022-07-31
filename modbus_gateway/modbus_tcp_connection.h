#ifndef MODBUS_GATEWAY_MODBUS_TCP_CONNECTION_H
#define MODBUS_GATEWAY_MODBUS_TCP_CONNECTION_H

#include <exchange/actor_helper.h>
#include <messages/modbus_message.h>
#include <types.h>
#include <common/synchronized.h>

namespace modbus_gateway
{

class ModbusTcpConnection final: public exchange::ActorHelper< ModbusTcpConnection >
{
     using ModbusMessageInfoOpt =  std::optional< ModbusMessageInfo >;

public:
     explicit ModbusTcpConnection( exchange::ActorId serverId, TcpSocketPtr socket, RouterPtr  router );

     ~ModbusTcpConnection() override = default;

     void Receive( const exchange::MessagePtr& ) override;

     void Start();

     void Stop();

private:
     static ModbusMessagePtr MakeRequest( const ModbusBufferPtr& modbusBuffer, size_t size, exchange::ActorId masterId );

     void StartReadTask();

     ModbusBufferPtr MakeResponse( const ModbusMessagePtr& modbusMessage );

     void StartWriteMessage( const ModbusMessagePtr& modbusMessage );

private:
     exchange::ActorId serverId_;
     TcpSocketPtr socket_;
     RouterPtr router_;
     Synchronized< ModbusMessageInfoOpt > syncRequestInfo_;
};

}


#endif
