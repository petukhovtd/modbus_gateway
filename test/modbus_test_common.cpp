#include <modbus_test_common.h>

namespace modbus::test
{

modbus::ModbusBuffer MakeModbusBuffer( const AduBuffer& frame, FrameType type )
{
     ModbusBuffer modbusBuffer( type );
     std::copy( frame.begin(), frame.end(), modbusBuffer.begin() );
     modbusBuffer.SetAduSize( frame.size() );
     return modbusBuffer;
}

}
