#include <modbus/modbus_buffer_rtu_wrapper.h>
#include <modbus/modbus_framing.h>
#include <sstream>

namespace modbus
{

ModbusBufferRtuWrapper::ModbusBufferRtuWrapper( ModbusBuffer& modbusBuffer )
          : modbusBuffer_( modbusBuffer )
{
     const FrameType frameType = modbusBuffer_.GetType();
     if( frameType != FrameType::RTU )
     {
          std::ostringstream os;
          os << "expect RTU modbus buffer, but is " << GetTypeName( frameType );
          throw std::logic_error( os.str() );
     }
}

void ModbusBufferRtuWrapper::UpdateCrc()
{
     const auto begin = modbusBuffer_.begin();
     auto end = modbusBuffer_.end() - crcSize;
     uint16_t crc = CalculateCrc( begin, end );
     const auto pair = U16ToBuffer( crc );
     // Lo Hi bytes
     *end = pair.second;
     ++end;
     *end = pair.first;
}

}
