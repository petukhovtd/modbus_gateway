#ifndef MODBUS_MODBUS_BUFFER_TCP_WRAPPER_H
#define MODBUS_MODBUS_BUFFER_TCP_WRAPPER_H

#include <modbus/modbus_buffer.h>

namespace modbus
{

/// @brief Обертка TCP фрейма
class ModbusBufferTcpWrapper: public IModbusBufferWrapper
{
public:
     /// @brief Конструктор класса
     /// @param[in,out] modbusBuffer
     /// @throw std::logic_error если фрейм не TCP типа
     explicit ModbusBufferTcpWrapper( ModbusBuffer& modbusBuffer );

     ~ModbusBufferTcpWrapper() override = default;

     /// @brief Получить номер транзакции
     /// @return
     TransactionId GetTransactionId() const;

     /// @brief Установить номер транзакции
     /// @param[in] trId
     void SetTransactionId( TransactionId trId );

     /// @brief Получить номер протокола
     /// @return
     ProtocolId GetProtocolId() const;

     /// @brief Получить длину фрейма
     /// @return
     Length GetLength() const;

     /// @brief Проверка номера протокола и длины
     /// @return
     CheckFrameResult Check() const override;

     /// @brief Обновление номера протокола и длины
     void Update() override;

private:
     ModbusBuffer& modbusBuffer_;
};

}

#endif
