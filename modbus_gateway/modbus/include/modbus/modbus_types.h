#ifndef MODBUS_GATEWAY_MODBUS_TYPES_H
#define MODBUS_GATEWAY_MODBUS_TYPES_H

#include <memory>
#include <vector>
#include <cstdint>

namespace modbus
{

enum FrameType
{
     RTU,
     ASCII,
     TCP
};

// Rtu
static const size_t unitIdSize = 1;
static const size_t functionCodeSize = 1;
static const size_t crcSize = 2;

static const size_t aduRtuMaxSize = 256; // 256 serial implementation limit

static const size_t rtuOverhead = unitIdSize + crcSize;
static_assert( rtuOverhead == 3 );
static const size_t pduMaxSize = aduRtuMaxSize - rtuOverhead;
static_assert( pduMaxSize == 253 );

static const size_t dataMaxSize = pduMaxSize - functionCodeSize;
static_assert( dataMaxSize == 252 );

// Tcp
static const size_t transactionIdSize = 2;
static const size_t protocolIdSize = 2;
static const size_t lengthSize = 2;

static const size_t mbapSize = transactionIdSize + protocolIdSize + lengthSize + unitIdSize;
static_assert( mbapSize == 7 );

static const size_t aduTcpMaxSize = pduMaxSize + mbapSize;
static_assert( aduTcpMaxSize == 260 );

// Ascii
static const uint8_t asciiStart = 0x3A; // :
static const uint16_t asciiEnd = 0x0D0A; // CRLF
static const size_t asciiStartSize = 1;
static const size_t asciiLrcSize = 2;
static const size_t asciiEndSize = 2;
static const size_t asciiOverhead = asciiStartSize + unitIdSize * 2 + asciiLrcSize + asciiEndSize;
static_assert( asciiOverhead == 7 );

static const size_t aduAsciiMaxSize = pduMaxSize * 2 + asciiOverhead;
static_assert( aduAsciiMaxSize == 513 );

// | ----------------------- ADU ----------------------------------------------------|
//                                    | ----------- PDU ---------|
// |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 |
// TCP
// | tr id   | pr id   | len     | id | fc | data
//                               RTU
//                               | id | fc | data                | CRC (2) |
//                       ASCII      ^   ^----  ^-------
//                          |star| id      | fc      | data      | LRC (2) | end (2) |
// ASCII frame convertible to RTU type after received, and before sending

static const size_t aduTcpStart = 0; // Начало tcp фрейма
static const size_t aduRtuStart = transactionIdSize + protocolIdSize + lengthSize; // Начало rtu фрейма
static const size_t aduAsciiStart = aduRtuStart - asciiStartSize; // Начало ascii фрейма

// Размер буфера для хранения любого типа фрейма
static const size_t aduBufferMaxSize = std::max( aduAsciiMaxSize, std::max( aduTcpMaxSize, aduRtuMaxSize ) );
static_assert( aduBufferMaxSize == aduAsciiMaxSize );

/// Поля TCP
enum class TcpField
{
     TransactionId,
     ProtocolId,
     Length,
};

static const size_t transactionIdPos = aduTcpStart;
static const size_t protocolIdPos = transactionIdPos + transactionIdSize;
static const size_t lengthPos = protocolIdPos + protocolIdSize;

/// Общие поля
enum class CommonField
{
     UnitId,
     FunctionCode,
     DataStart,
};

/// @brief Позиции общих полей
static const size_t unitIdGeneralPos = lengthPos + lengthSize;
static const size_t functionCodeGeneralPos = unitIdGeneralPos + unitIdSize;
static const size_t functionCodeASCIIPos = unitIdGeneralPos + unitIdSize * 2;
static const size_t dataGeneralPos = functionCodeGeneralPos + functionCodeSize;
static const size_t dataASCIIPos = functionCodeASCIIPos + functionCodeSize * 2;

using AduElementType = uint8_t;
static const AduElementType aduDefaultValue = 0;
using AduBuffer = std::vector< AduElementType >;
using AduBufferPtr = std::unique_ptr< AduBuffer >;

/// @brief Создание буфера размера aduBufferMaxSize и заполнением aduDefaultValue
/// @return
AduBufferPtr MakeAduBuffer();

/// @brief Получить индекс начала записи фрейма заданного типа
/// @param[in] type
/// @return
size_t GetAduStart( FrameType type );

/// @brief Получить максимальный размер фрейма заданного типа
/// @param[in] type
/// @return
size_t GetAduMaxSize( FrameType type );

/// @brief Получить тествое представление типа
/// @param[in] type
/// @return
std::string GetTypeName( FrameType type );

/// @brief Получить позицию начала поля относительно начала общего буфера
/// @param[in] frameType
/// @param[in] commonField
/// @return
size_t GetPosForType( FrameType frameType, CommonField commonField );

/// @brief Получить позицию начала поля относительно начала общего буфера
/// @param[in] tcpField
/// @return
size_t GetPosForType( TcpField tcpField );

/// @brief Расчитать размер pdu
/// @param[in] type тип фремйма
/// @param[in] aduSize размер фрейма
/// @return
size_t CalculatePduSize( FrameType type, size_t aduSize );

/// @brief Расчитать размер adu
/// @param[in] type
/// @param[in] pduSize
/// @return
size_t CalculateAduSize( FrameType type, size_t pduSize );

}

#endif
