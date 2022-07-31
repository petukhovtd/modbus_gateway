#include <gtest/gtest.h>
#include <test/modbus_common.h>
#include <modbus/modbus_framing.h>

using namespace modbus;

TEST( ModbusFraming, U16FromBuffer )
{
     EXPECT_EQ( U16FromBuffer( 0x12, 0x34 ), 0x1234 );
     EXPECT_EQ( U16FromBuffer( 0x43, 0x21 ), 0x4321 );
     EXPECT_EQ( U16FromBuffer( 0, 0x1 ), 0x1 );
     EXPECT_EQ( U16FromBuffer( 0x10, 0 ), 0x1000 );
     EXPECT_EQ( U16FromBuffer( 0x1, 0 ), 0x0100 );
     EXPECT_EQ( U16FromBuffer( 0, 0 ), 0 );
}

TEST( ModbusFraming, U16ToBuffer )
{
     {
          const auto p = U16ToBuffer( 0x1234 );
          EXPECT_EQ( p.first, 0x12 );
          EXPECT_EQ( p.second, 0x34 );
     }
     {
          const auto p = U16ToBuffer( 0x4321 );
          EXPECT_EQ( p.first, 0x43 );
          EXPECT_EQ( p.second, 0x21 );
     }
     {
          const auto p = U16ToBuffer( 0x1000 );
          EXPECT_EQ( p.first, 0x10 );
          EXPECT_EQ( p.second, 0 );
     }
     {
          const auto p = U16ToBuffer( 0x1 );
          EXPECT_EQ( p.first, 0 );
          EXPECT_EQ( p.second, 0x1 );
     }
     {
          const auto p = U16ToBuffer( 0 );
          EXPECT_EQ( p.first, 0 );
          EXPECT_EQ( p.second, 0 );
     }
}

TEST( ModbusFraming, asciiToU4 )
{
     EXPECT_EQ( ascii::ToU4( 'A' ), 0xA );
     EXPECT_EQ( ascii::ToU4( 'B' ), 0xB );
     EXPECT_EQ( ascii::ToU4( 'C' ), 0xC );
     EXPECT_EQ( ascii::ToU4( 'D' ), 0xD );
     EXPECT_EQ( ascii::ToU4( 'E' ), 0xE );
     EXPECT_EQ( ascii::ToU4( 'F' ), 0xF );
     EXPECT_EQ( ascii::ToU4( '0' ), 0 );
     EXPECT_EQ( ascii::ToU4( '1' ), 1 );
     EXPECT_EQ( ascii::ToU4( '2' ), 2 );
     EXPECT_EQ( ascii::ToU4( '3' ), 3 );
     EXPECT_EQ( ascii::ToU4( '4' ), 4 );
     EXPECT_EQ( ascii::ToU4( '5' ), 5 );
     EXPECT_EQ( ascii::ToU4( '6' ), 6 );
     EXPECT_EQ( ascii::ToU4( '7' ), 7 );
     EXPECT_EQ( ascii::ToU4( '8' ), 8 );
     EXPECT_EQ( ascii::ToU4( '9' ), 9 );
}

TEST( ModbusFraming, asciiFromU4 )
{
     EXPECT_EQ( ascii::FromU4( 0 ), '0' );
     EXPECT_EQ( ascii::FromU4( 1 ), '1' );
     EXPECT_EQ( ascii::FromU4( 2 ), '2' );
     EXPECT_EQ( ascii::FromU4( 3 ), '3' );
     EXPECT_EQ( ascii::FromU4( 4 ), '4' );
     EXPECT_EQ( ascii::FromU4( 5 ), '5' );
     EXPECT_EQ( ascii::FromU4( 6 ), '6' );
     EXPECT_EQ( ascii::FromU4( 7 ), '7' );
     EXPECT_EQ( ascii::FromU4( 8 ), '8' );
     EXPECT_EQ( ascii::FromU4( 9 ), '9' );
     EXPECT_EQ( ascii::FromU4( 0xA ), 'A' );
     EXPECT_EQ( ascii::FromU4( 0xB ), 'B' );
     EXPECT_EQ( ascii::FromU4( 0xC ), 'C' );
     EXPECT_EQ( ascii::FromU4( 0xD ), 'D' );
     EXPECT_EQ( ascii::FromU4( 0xE ), 'E' );
     EXPECT_EQ( ascii::FromU4( 0xF ), 'F' );
}

TEST( ModbusFraming, asciiToU8 )
{
     EXPECT_EQ( ascii::ToU8( 'A', 'B' ), 0xAB );
     EXPECT_EQ( ascii::ToU8( '3', '6' ), 0x36 );
     EXPECT_EQ( ascii::ToU8( 'D', '5' ), 0xD5 );
     EXPECT_EQ( ascii::ToU8( '2', 'F' ), 0x2F );
     EXPECT_EQ( ascii::ToU8( 'F', 'F' ), 0xFF );
     EXPECT_EQ( ascii::ToU8( '0', '0' ), 0 );
     EXPECT_EQ( ascii::ToU8( '0', '5' ), 0x5 );
     EXPECT_EQ( ascii::ToU8( '9', '0' ), 0x90 );
}

TEST( ModbusFraming, asciiFromU8 )
{
     {
          const auto p = ascii::FromU8( 0x12 );
          EXPECT_EQ( p.first, '1' );
          EXPECT_EQ( p.second, '2' );
     }
     {
          const auto p = ascii::FromU8( 0x2F );
          EXPECT_EQ( p.first, '2' );
          EXPECT_EQ( p.second, 'F' );
     }
     {
          const auto p = ascii::FromU8( 0xD8 );
          EXPECT_EQ( p.first, 'D' );
          EXPECT_EQ( p.second, '8' );
     }
     {
          const auto p = ascii::FromU8( 0xAC );
          EXPECT_EQ( p.first, 'A' );
          EXPECT_EQ( p.second, 'C' );
     }
     {
          const auto p = ascii::FromU8( 0x5 );
          EXPECT_EQ( p.first, '0' );
          EXPECT_EQ( p.second, '5' );
     }
     {
          const auto p = ascii::FromU8( 0x90 );
          EXPECT_EQ( p.first, '9' );
          EXPECT_EQ( p.second, '0' );
     }
     {
          const auto p = ascii::FromU8( 0 );
          EXPECT_EQ( p.first, '0' );
          EXPECT_EQ( p.second, '0' );
     }
}

TEST( ModbusFraming, asciiToAscii )
{
     using namespace modbus;
     {
          AduBuffer binary = { 0x12, 0x34 };
          EXPECT_THROW( ascii::ToAscii( binary.begin(), binary.begin(), binary.size() ),
                        std::logic_error );
          EXPECT_THROW( ascii::ToAscii( binary.begin() + 1, binary.begin(), binary.size() ),
                        std::logic_error );
     }
     {
          static const size_t shift = 1;
          AduBuffer binary = { 0x12, 0x5A, 0x08 };
          static const AduBuffer asciiReference = { 0x12, '1', '2', '5', 'A', '0', '8' };
          size_t size = binary.size();
          binary.resize( binary.size() * 2 + 1 );
          ascii::ToAscii( binary.begin(), binary.begin() + shift, size );
          EXPECT_TRUE( test::Compare( binary.cbegin(), binary.cend(), asciiReference.begin(), asciiReference.end() ) );
     }
}

TEST( ModbusFraming, asciiFromAscii )
{
     using namespace modbus;
     {
          AduBuffer ascii = { '1', '2', '5', 'A', '0', '8' };
          EXPECT_THROW( ascii::FromAscii( ascii.begin(), ascii.begin(), ascii.size() ), std::logic_error );
          EXPECT_THROW( ascii::FromAscii( ascii.begin(), ascii.begin() + 1, ascii.size() ), std::logic_error );
     }
     {
          AduBuffer ascii = { 0x0, '1', '2', '5', 'A', '0', '8' };
          static const AduBuffer binaryReference = { 0x12, 0x5A, 0x08, '5', 'A', '0', '8' };
          ascii::FromAscii( ascii.begin() + 1, ascii.begin(), ascii.size() - 1 );
          EXPECT_TRUE( test::Compare( ascii.cbegin(), ascii.cend(), binaryReference.begin(), binaryReference.end() ) );
     }
}

TEST( ModbusFraming, asciiCalculateLrc )
{
     using namespace modbus;
     {
          static const AduBuffer buf = { 0x10, 0x03, 0x02, 0xBC, 0x00, 0x02 };
          EXPECT_EQ( ascii::CalculateLrc( buf.begin(), buf.end() ), 0x2D );
     }
     {
          static const AduBuffer buf = { 0x10, 0x03, 0x00, 0x0A, 0x00, 0x01 };
          EXPECT_EQ( ascii::CalculateLrc( buf.begin(), buf.end() ), 0xE2 );
     }
}

TEST( ModbusFraming, CalculateCrc )
{
     using namespace modbus;
     {
          static const AduBuffer buf = { 0x01 };
          EXPECT_EQ( CalculateCrc( buf.begin(), buf.end() ), 0x807E );
     }
     {
          static const AduBuffer buf = { 0x02, 0x07 };
          EXPECT_EQ( CalculateCrc( buf.begin(), buf.end() ), 0x1241 );
     }
     {
          static const AduBuffer buf = { 0x03, 0x09, 0xAD };
          EXPECT_EQ( CalculateCrc( buf.begin(), buf.end() ), 0x2D46 );
     }
}
