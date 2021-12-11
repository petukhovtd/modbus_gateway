#include "modbus/modbus_framing.h"

namespace modbus
{

uint16_t U16FromBuffer( uint8_t first, uint8_t second )
{
     return ( static_cast< uint16_t >( first ) << 8u ) + second;
}

std::pair< uint8_t, uint8_t > U16ToBuffer( uint16_t value )
{
     return { value >> 8u, value & 0xFFu };
}

namespace ascii
{

uint8_t ToU4( unsigned char c )
{
     if( c < 'A' )
     {
          return c - '0';
     }
     // c - 'A' + 10
     return c - '7';
}

unsigned char FromU4( uint8_t u )
{
     if( u < 10 )
     {
          return '0' + u;
     }
     // u - 10 + 'A'
     return u + '7';
}

uint8_t ToU8( unsigned char first, unsigned char second )
{
     return ( ToU4( first ) << 4u ) + ToU4( second );
}


std::pair< unsigned char, unsigned char > FromU8( uint8_t u )
{
     return { FromU4( u >> 4u ), FromU4( u & 0x0Fu ) };
}

void ToAscii( modbus::AduBuffer::iterator from, modbus::AduBuffer::iterator to, size_t size )
{
     if( std::distance( from, to ) <= 0 )
     {
          throw std::logic_error( "from <= to, expect to > from" );
     }
     modbus::AduBuffer tmp( from, from + size );
     for( const modbus::AduBuffer::value_type value: tmp )
     {
          const auto pair = FromU8( value );
          *to = pair.first;
          ++to;
          *to = pair.second;
          ++to;
     }
}

void FromAscii( modbus::AduBuffer::iterator from, modbus::AduBuffer::iterator to, size_t size )
{
     if( std::distance( from, to ) >= 0 )
     {
          throw std::logic_error( "from >= to, expect to < from" );
     }
     const auto fromEnd = from + size;
     for( ; from != fromEnd; ++from )
     {
          modbus::AduBuffer::value_type first = *from;
          ++from;
          modbus::AduBuffer::value_type second = *from;
          *to = ToU8( first, second );
          ++to;
     }
}

}
}
