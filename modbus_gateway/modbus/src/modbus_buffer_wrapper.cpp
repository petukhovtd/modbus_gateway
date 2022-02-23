#include <modbus/modbus_buffer_wrapper.h>
#include <modbus/modbus_buffer_rtu_wrapper.h>
#include <modbus/modbus_buffer_tcp_wrapper.h>
#include <modbus/modbus_buffer_ascii_wrapper.h>

namespace modbus
{

std::shared_ptr< IModbusBufferWrapper > MakeModbusBufferWrapper( ModbusBuffer& modbusBuffer )
{
     switch( modbusBuffer.GetType() )
     {
          case RTU:
               return std::make_shared< ModbusBufferRtuWrapper >( modbusBuffer );
          case ASCII:
               return std::make_shared< ModbusBufferAsciiWrapper >( modbusBuffer );
          case TCP:
               return std::make_shared< ModbusBufferTcpWrapper >( modbusBuffer );
          default:
               throw std::logic_error( "invalid frame type" );
     }
}

}
