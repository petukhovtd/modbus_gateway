#include <modbus_test_common.h>

namespace modbus::test
{

modbus::ModbusBuffer MakeModbusBuffer( const AduBuffer& frame, FrameType type )
{
     ModbusBuffer modbusBuffer( type );
     std::copy( frame.begin(), frame.end(), modbusBuffer.begin() );
     if( !modbusBuffer.SetAduSize( frame.size() ) )
     {
          throw std::logic_error( "invalid adu size" );
     }
     return modbusBuffer;
}

}
