#include <iostream>
#include <iomanip>

#include <common/fmt_logger.h>
#include <types.h>
#include <tcp_server.h>
#include <modbus_tcp_slave.h>

#include <exchange/exchange.h>

using namespace modbus_gateway;

uint8_t ModbusASCIIToU8( unsigned char c )
{
     if( c < 'A' )
     {
          return c - '0';
     }
     // c - 'A' + 10
     return c - '7';
}

uint8_t ModbusASCIIToU8( unsigned char first, unsigned char second )
{
     return ( ModbusASCIIToU8( first ) << 4u ) + ModbusASCIIToU8( second );
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

std::pair< unsigned char, unsigned char > FromU8( uint8_t u )
{
     return { FromU4( u >> 4u ), FromU4( u & 0x0Fu ) };
}

uint16_t U16FromBuffer( uint8_t first, uint8_t second )
{
     return second + ( first << 8u );
}

int main()
{
     std::cout << (int) ModbusASCIIToU8( '0' ) << std::endl;
     std::cout << (int) ModbusASCIIToU8( '1' ) << std::endl;
     std::cout << (int) ModbusASCIIToU8( '2' ) << std::endl;
     std::cout << (int) ModbusASCIIToU8( 'A' ) << std::endl;
     std::cout << (int) ModbusASCIIToU8( 'C' ) << std::endl;
     std::cout << (int) ModbusASCIIToU8( 'F', 'F' ) << std::endl;
     std::cout << (int) ModbusASCIIToU8( '0', 'F' ) << std::endl;
     std::cout << (int) ModbusASCIIToU8( 'F', '0' ) << std::endl;
     std::cout << std::hex << U16FromBuffer( 0x01, 0x02 ) << std::endl;
     return 0;

     FmtLogger::SetLogLevel( FmtLogger::LogLevel::Trace );
     ContextPtr context = std::make_shared< ContextPtr::element_type >();
     TcpServer::Ptr tcpServer = TcpServer::Create( context, 1234 );
     exchange::Exchange::Insert( tcpServer );
     tcpServer->Start();
     context->run();
     return 0;
}