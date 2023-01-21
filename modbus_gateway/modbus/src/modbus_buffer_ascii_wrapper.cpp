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

CheckFrameResult ModbusBufferAsciiWrapper::Check() const
{
     auto begin = modbusBuffer_.begin();
     if( *begin != asciiStart )
     {
          return CheckFrameResult::AsciiInvalidStartTag;
     }
     begin += asciiStartSize;

     auto end = modbusBuffer_.end() - asciiEndSize;
     const auto asciiEndPair = U16ToBuffer( asciiEnd );
     if( *end != asciiEndPair.first || *( end + 1 ) != asciiEndPair.second )
     {
          return CheckFrameResult::AsciiInvalidEndTag;
     }
     end -= asciiLrcSize;

     uint8_t lrc = ascii::CalculateLrc( begin, end );
     const auto lrcPair = ascii::FromU8( lrc );
     if( *end != lrcPair.first || *( end + 1 ) != lrcPair.second )
     {
          return CheckFrameResult::AsciiInvalidLrc;
     }

     return CheckFrameResult::NoError;
}

void ModbusBufferAsciiWrapper::Update()
{
     auto begin = modbusBuffer_.begin();
     *begin = asciiStart;

     begin += asciiStartSize;

     auto end = modbusBuffer_.end() - asciiEndSize;
     const auto asciiEndPair = U16ToBuffer( asciiEnd );
     *end = asciiEndPair.first;
     *( end + 1 ) = asciiEndPair.second;

     end -= asciiLrcSize;

     uint8_t lrc = ascii::CalculateLrc( begin, end );
     const auto lrcPair = ascii::FromU8( lrc );
     *end = lrcPair.first;
     *( end + 1 ) = lrcPair.second;
}

}
