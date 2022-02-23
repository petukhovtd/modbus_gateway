#ifndef MODBUS_MODBUS_BUFFER_WRAPPER_H
#define MODBUS_MODBUS_BUFFER_WRAPPER_H

#include <modbus/modbus_buffer.h>

namespace modbus
{

/// @brief Создать обертку из буфера
/// @param[in,out] modbusBuffer
/// @return
std::shared_ptr< IModbusBufferWrapper > MakeModbusBufferWrapper( ModbusBuffer& modbusBuffer );

}

#endif
