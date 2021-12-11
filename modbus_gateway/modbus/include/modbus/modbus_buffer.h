#ifndef MODBUS_GATEWAY_MODBUS_BUFFER_H
#define MODBUS_GATEWAY_MODBUS_BUFFER_H

#include "modbus_types.h"

namespace modbus
{

/// @brief Буфер протокола modbus
/// при создании аллоцирует максимальный требуемый буфер подходящий под любой тип протокола
/// и позволяет проводить преобразование типов без копирования PDU и выделения новой памяти
/// за исключением ASCII, т.к. требует разделения числа на текст и наоборот
class ModbusBuffer
{
public:
     /// @brief Создание буфера указанного типа фрейма
     /// @param[in] type тип фрейма
     explicit ModbusBuffer( FrameType type );

     ModbusBuffer( ModbusBuffer& ) = delete;

     /// @brief Получить тип фрейма
     /// @return
     FrameType GetType() const;

     /// @brief Начало фрейма
     /// @return
     AduBuffer::iterator begin();

     /// @brief Начало фрейма
     /// @return
     AduBuffer::const_iterator begin() const;

     /// @brief Конец фрейма (от начала фрейма до adu size)
     /// @return
     AduBuffer::iterator end();

     /// @brief Конец фрейма (от начала фрейма до adu size)
     /// @return
     AduBuffer::const_iterator end() const;

     /// @brief Начало фрейма
     /// @return
     AduBuffer::const_iterator cbegin() const;

     /// @brief Конец фрейма (от начала фрейма до adu size)
     /// @return
     AduBuffer::const_iterator cend() const;

     /// @brief Установить adu size, по умолчанию 0
     /// @return
     void SetAduSize( size_t aduSize );

     /// @brief Получить adu size
     /// @return
     size_t GetAduSize() const;

     /// @brief Получить unit id
     /// @return
     uint8_t GetUnitId() const;

     /// @brief Установить unit id
     /// @param id
     void SetUnitId( uint8_t id );

     /// @brief Получить fc
     /// @return
     uint8_t GetFunctionCode() const;

     /// @brief Установить fc
     /// @param fc
     void SetFunctionCode( uint8_t fc );

     /// @brief Конвертация фрема в другой формат
     /// TCP<->RTU изменяется только указатели на начало и конец буфера и размер
     /// ASCII<->[TCP,RTU] производит конвертацию, что может аллоцировать дополнительную память
     /// записывает стартовый и конечный маркер
     /// !!! не обновляет и не проверяет контрольные суммы для RTU и ASCII фреймов
     /// @param[in] toType
     void ConvertTo( FrameType toType );

private:
     FrameType type_;
     AduBufferPtr buffer_;
     size_t aduSize_;
};

}

#endif
