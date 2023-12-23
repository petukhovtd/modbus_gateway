#ifndef MODBUS_GATEWAY_MODBUS_TCP_MASTER_H
#define MODBUS_GATEWAY_MODBUS_TCP_MASTER_H

#include <types.h>
#include <common/limit_queue.h>
#include <messages/modbus_message.h>

#include <exchange/actor_helper.h>

namespace modbus_gateway
{

class ModbusTcpMaster: public exchange::ActorHelper< ModbusTcpMaster >
{
     struct ModbusCurrentMessage
     {
          ModbusMessagePtr modbusMessage;
          modbus::TransactionId id;
     };

     enum class State
     {
          Idle,
          WaitConnect,
          Connected,
          MessageProcess,
     };


public:
     ModbusTcpMaster( const ContextPtr& context, const asio::ip::address& addr, asio::ip::port_type port, std::chrono::milliseconds timeout );

     ~ModbusTcpMaster() override;

     void Receive( const exchange::MessagePtr& message ) override;

private:
     void MessageProcess( const ModbusMessagePtr& message );

     void QueueProcessUnsafe();

     void StartConnectTaskUnsafe();

     void StartMessageTaskUnsafe();

     void StartWaitTask();

     ModbusMessagePtr MakeResponse( const ModbusBufferPtr& modbusBuffer, size_t size );

     void StartReceiveTask();

     static std::string StateToStr( State state );

private:
     TcpSocketUPtr socket_;
     asio::ip::tcp::endpoint ep_;
     TimeoutMs timeout_;
     asio::basic_waitable_timer< std::chrono::steady_clock > timer_;
     std::mutex m_;
     LimitQueue< ModbusMessagePtr > messageQueue_;
     std::optional< ModbusCurrentMessage > currentMessage_;
     modbus::TransactionId transactionIdGenerator_;
     State state_;
};

}

#endif //MODBUS_GATEWAY_MODBUS_TCP_MASTER_H
