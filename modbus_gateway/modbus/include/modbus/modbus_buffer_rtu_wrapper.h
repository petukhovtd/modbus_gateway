#ifndef MODBUS_GATEWAY_MODBUS_BUFFER_RTU_WRAPPER_H
#define MODBUS_GATEWAY_MODBUS_BUFFER_RTU_WRAPPER_H

#include <modbus/modbus_buffer.h>

namespace modbus
{

/// @brief Обертка RTU фрейма
class ModbusBufferRtuWrapper
{
public:
     /// @brief Конструктор класса
     /// @param[in,out] modbusBuffer
     /// @throw std::logic_error если фрейм не RTU типа
     explicit ModbusBufferRtuWrapper( ModbusBuffer& modbusBuffer );

     /// @brief Обновить значение CRC
     void UpdateCrc();

private:
     ModbusBuffer& modbusBuffer_;
};


}

#endif
