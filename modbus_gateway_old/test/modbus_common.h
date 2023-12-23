#ifndef MODBUS_GATEWAY_MODBUS_COMMON_H
#define MODBUS_GATEWAY_MODBUS_COMMON_H

#include <modbus/modbus_buffer.h>

namespace test
{

template< typename It >
bool Compare( It beginLhs, It endLhs, It beginRhs, It endRhs )
{
     if( std::distance( beginLhs, endLhs ) != std::distance( beginRhs, endRhs ) )
     {
          return false;
     }

     for( ; beginLhs != endLhs; ++beginLhs, ++beginRhs )
     {
          if( *beginLhs != *beginRhs )
          {
               return false;
          }
     }
     return true;
}

template< typename Container >
bool Compare( const Container& lhs, const Container& rhs )
{
     return Compare( std::begin( lhs ), std::end( lhs ), std::begin( rhs ), std::end( rhs ) );
}

modbus::ModbusBuffer MakeModbusBuffer( const modbus::AduBuffer& frame, modbus::FrameType type );

}

#endif
