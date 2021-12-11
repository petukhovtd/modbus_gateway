#include "modbus/modbus_buffer_tcp_wrapper.h"
#include "modbus/modbus_framing.h"

#include <sstream>

namespace modbus
{

namespace
{

uint16_t TcpGetter( TcpField field, AduBuffer::const_iterator begin )
{
     const size_t pos = GetPosForType( field );
     const auto it = begin + pos;
     return U16FromBuffer( *it, *( it + 1 ) );
}

void TcpSetter( TcpField field, AduBuffer::iterator begin, uint16_t value )
{
     const auto res = U16ToBuffer( value );
     auto it = begin + GetPosForType( field );
     *it = res.first;
     ++it;
     *it = res.second;
}

}

ModbusBufferTcpWrapper::ModbusBufferTcpWrapper( ModbusBuffer& modbusBuffer )
          : modbusBuffer_( modbusBuffer )
{
     const FrameType frameType = modbusBuffer_.GetType();
     if( frameType != FrameType::TCP )
     {
          std::ostringstream os;
          os << "expect TCP modbus buffer, but is " << GetTypeName( frameType );
          throw std::logic_error( os.str() );
     }
}

uint16_t ModbusBufferTcpWrapper::GetTransactionId() const
{
     return TcpGetter( TcpField::TransactionId, modbusBuffer_.begin() );
}

void ModbusBufferTcpWrapper::SetTransactionId( uint16_t trId )
{
     TcpSetter( TcpField::TransactionId, modbusBuffer_.begin(), trId );
}

uint16_t ModbusBufferTcpWrapper::GetProtocolId() const
{
     return TcpGetter( TcpField::ProtocolId, modbusBuffer_.begin() );
}

void ModbusBufferTcpWrapper::SetProtocolId( uint16_t protoId )
{
     TcpSetter( TcpField::ProtocolId, modbusBuffer_.begin(), protoId );
}

uint16_t ModbusBufferTcpWrapper::GetLength() const
{
     return TcpGetter( TcpField::Length, modbusBuffer_.begin() );
}

void ModbusBufferTcpWrapper::SetLength( uint16_t length )
{
     TcpSetter( TcpField::Length, modbusBuffer_.begin(), length );
}

}
