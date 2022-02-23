#ifndef MODBUS_MODBUS_BUFFER_RTU_WRAPPER_H
#define MODBUS_MODBUS_BUFFER_RTU_WRAPPER_H

#include <modbus/modbus_buffer.h>

namespace modbus
{

/// @brief Обертка RTU фрейма
class ModbusBufferRtuWrapper: public IModbusBufferWrapper
{
public:
     /// @brief Конструктор класса
     /// @param[in,out] modbusBuffer
     /// @throw std::logic_error если фрейм не RTU типа
     explicit ModbusBufferRtuWrapper( ModbusBuffer& modbusBuffer );

     ~ModbusBufferRtuWrapper() override = default;

     /// @brief Проверяет crc
     /// @return
     CheckFrameResult Check() const override;

     /// @brief Обновляет crc
     void Update() override;

private:
     ModbusBuffer& modbusBuffer_;
};


}

#endif
