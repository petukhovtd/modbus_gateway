#ifndef MODBUS_GATEWAY_MODBUS_BUFFER_ASCII_WRAPPER_H
#define MODBUS_GATEWAY_MODBUS_BUFFER_ASCII_WRAPPER_H

#include <modbus/modbus_buffer.h>

namespace modbus
{

/// @brief Обертка ASCII фрейма
class ModbusBufferAsciiWrapper
{
public:
     /// @brief Конструктор класса
     /// @param[in,out] modbusBuffer
     /// @throw std::logic_error если фрейм не ASCII типа
     explicit ModbusBufferAsciiWrapper( ModbusBuffer& modbusBuffer );

     /// @brief Обновить значение LRC
     void UpdateLrc();

private:
     ModbusBuffer& modbusBuffer_;
};

}

#endif
