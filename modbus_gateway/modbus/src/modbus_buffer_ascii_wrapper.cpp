#include <modbus/modbus_buffer_ascii_wrapper.h>
#include <modbus/modbus_framing.h>
#include <sstream>

namespace modbus
{

ModbusBufferAsciiWrapper::ModbusBufferAsciiWrapper( ModbusBuffer& modbusBuffer )
: modbusBuffer_( modbusBuffer )
{
     const FrameType frameType = modbusBuffer_.GetType();
     if( frameType != FrameType::ASCII )
     {
          std::ostringstream os;
          os << "expect ASCII modbus buffer, but is " << GetTypeName( frameType );
          throw std::logic_error( os.str() );
     }
}

void ModbusBufferAsciiWrapper::UpdateLrc()
{
     const auto begin = modbusBuffer_.begin() + asciiStartSize;
     auto end = modbusBuffer_.end() - asciiEndSize - asciiLrcSize;
     uint8_t lrc = ascii::CalculateLrc( begin, end );
     const auto pair = ascii::FromU8( lrc );
     *end = pair.first;
     ++end;
     *end = pair.second;
}

}
