#include "modbus/modbus_buffer.h"
#include "modbus/modbus_framing.h"

#include <sstream>

namespace modbus
{

namespace
{

/// @brief Общая функция получения значения в буфере
/// @param[in] frameType тип фрейма
/// @param[in] field тип поля
/// @param[in] begin начало буфера
/// @return
uint8_t CommonGetter( FrameType frameType, CommonField field, AduBuffer::const_iterator begin )
{
     auto it = begin + GetPosForType( frameType, field );
     switch( frameType )
     {
          case RTU:
          case TCP:
               return *it;
          case ASCII:
               return modbus::ascii::ToU8( *( it ), *( it + 1 ) );
          default:
               return 0;
     }
}

/// @brief Общая функция устанлвки значения в буфере
/// @param[in] frameType тип фрейма
/// @param[in] field тип поля
/// @param[in] begin начало буфера
/// @param[in] value значение
/// @return
void CommonSetter( FrameType frameType, CommonField field, AduBuffer::iterator begin, uint8_t value )
{
     auto it = begin + GetPosForType( frameType, field );
     switch( frameType )
     {
          case RTU:
          case TCP:
               *it = value;
               break;
          case ASCII:
          {
               auto res = modbus::ascii::FromU8( value );
               *it = res.first;
               ++it;
               *it = res.second;
          }
               break;
          default:
               break;
     }
}

}

ModbusBuffer::ModbusBuffer( FrameType type )
          : type_( type )
          , buffer_( MakeAduBuffer() )
          , aduSize_( GetAduMaxSize( type ) )
{}

FrameType ModbusBuffer::GetType() const
{
     return type_;
}

AduBuffer::iterator ModbusBuffer::begin()
{
     return buffer_.begin() + GetAduStart( type_ );
}

AduBuffer::const_iterator ModbusBuffer::begin() const
{
     return buffer_.begin() + GetAduStart( type_ );
}

AduBuffer::iterator ModbusBuffer::end()
{
     return ModbusBuffer::begin() + aduSize_;
}

AduBuffer::const_iterator ModbusBuffer::end() const
{
     return ModbusBuffer::begin() + aduSize_;
}

AduBuffer::const_iterator ModbusBuffer::cbegin() const
{
     return buffer_.begin() + GetAduStart( type_ );
}

AduBuffer::const_iterator ModbusBuffer::cend() const
{
     return ModbusBuffer::begin() + aduSize_;
}

void ModbusBuffer::SetAduSize( size_t aduSize )
{
     if( aduSize > GetAduMaxSize( type_ ) )
     {
          std::ostringstream os;
          os << "invalid adu size: " << aduSize << " for type " << GetTypeName( type_ ) << ", max size "
             << GetAduMaxSize( type_ );
          throw std::logic_error( os.str() );
     }
     aduSize_ = aduSize;
}

size_t ModbusBuffer::GetAduSize() const
{
     return aduSize_;
}

uint8_t ModbusBuffer::GetUnitId() const
{
     return CommonGetter( type_, CommonField::UnitId, buffer_.begin() );
}

void ModbusBuffer::SetUnitId( uint8_t id )
{
     return CommonSetter( type_, CommonField::UnitId, buffer_.begin(), id );
}

uint8_t ModbusBuffer::GetFunctionCode() const
{
     return CommonGetter( type_, CommonField::FunctionCode, buffer_.begin() );
}

void ModbusBuffer::SetFunctionCode( uint8_t fc )
{
     return CommonSetter( type_, CommonField::FunctionCode, buffer_.begin(), fc );
}

void ModbusBuffer::ConvertTo( FrameType toType )
{
     if( type_ == toType )
     {
          return;
     }

     uint8_t unitId = GetUnitId();
     size_t pduSize = CalculatePduSize( type_, aduSize_ );

     if( pduSize >= aduSize_ )
     {
          throw std::logic_error( "pdu size more then adu size" );
     }

     // TCP -> ASCII
     // RTU -> ASCII
     if( toType == FrameType::ASCII )
     {
          ascii::ToAscii( buffer_.begin() + functionCodeGeneralPos,
                          buffer_.begin() + functionCodeASCIIPos,
                          pduSize );
          pduSize *= 2;

          auto asciiBeginIt = buffer_.begin() + GetAduStart( FrameType::ASCII );
          *asciiBeginIt = asciiStart;

          auto asciiEndIt = asciiBeginIt + CalculateAduSize( FrameType::ASCII, pduSize ) - asciiEndSize;
          const auto pair = U16ToBuffer( asciiEnd );
          *asciiEndIt = pair.first;
          ++asciiEndIt;
          *asciiEndIt = pair.second;
     }

     // ASCII -> RTU
     // ASCII -> TCP
     if( type_ == FrameType::ASCII )
     {
          ascii::FromAscii( buffer_.begin() + functionCodeASCIIPos,
                            buffer_.begin() + functionCodeGeneralPos,
                            pduSize );
          pduSize /= 2;
     }

     type_ = toType;
     aduSize_ = CalculateAduSize( type_, pduSize );
     SetUnitId( unitId );
}


}
