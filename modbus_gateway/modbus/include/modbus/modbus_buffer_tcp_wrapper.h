#ifndef MODBUS_GATEWAY_MODBUS_BUFFER_TCP_WRAPPER_H
#define MODBUS_GATEWAY_MODBUS_BUFFER_TCP_WRAPPER_H

#include "modbus/modbus_buffer.h"

namespace modbus
{

/// @brief Обертка TCP фрейма
class ModbusBufferTcpWrapper
{
public:
     /// @brief Конструктор класса
     /// @param[in,out] modbusBuffer
     /// @throw std::logic_error если фрейм не TCP типа
     explicit ModbusBufferTcpWrapper( ModbusBuffer& modbusBuffer );

     /// @brief Получить номер транзакции
     /// @return
     uint16_t GetTransactionId() const;

     /// @brief Установить номер транзакции
     /// @param[in] trId
     void SetTransactionId( uint16_t trId );

     /// @brief Получить номер протокола
     /// @return
     uint16_t GetProtocolId() const;

     /// @brief Установить номер протокола
     /// @param[in] protoId
     void SetProtocolId( uint16_t protoId );

     /// @brief Получить длину фрейма
     /// @return
     uint16_t GetLength() const;

     /// @brief Установить длину фрейма
     /// @param[in] length
     void SetLength( uint16_t length );

private:
     ModbusBuffer& modbusBuffer_;
};

}

#endif
