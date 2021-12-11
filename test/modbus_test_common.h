#ifndef MODBUS_GATEWAY_TEST_COMMON_H
#define MODBUS_GATEWAY_TEST_COMMON_H

namespace modbus::test
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

}

#endif
