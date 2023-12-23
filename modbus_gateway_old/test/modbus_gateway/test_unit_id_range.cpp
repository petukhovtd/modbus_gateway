#include <gtest/gtest.h>
#include <config/unit_id_range.h>

TEST( UnitIdRangeTest, CompressValueToValueTest )
{
     namespace mg = modbus_gateway;
     using UnitIdRangeSet = std::vector< mg::UnitIdRange >;
     auto testHelper = []( UnitIdRangeSet input, const UnitIdRangeSet& expect )
     {
          mg::CompressUnitIdRanges( input );
          ASSERT_EQ( input.size(), expect.size() );
          for( size_t index = 0; index < input.size(); ++index )
          {
               EXPECT_EQ( input[ index ].begin, expect[ index ].begin );
               EXPECT_EQ( input[ index ].end, expect[ index ].end );
          }
     };

     // 1, 3 -> 1, 3
     testHelper( {
                 mg::UnitIdRange( 1 ),
                 mg::UnitIdRange( 3 )
                 },
                 {
                 mg::UnitIdRange( 1 ),
                 mg::UnitIdRange( 3 )
                 } );
     // 4, 1 -> 1, 4
     testHelper( {
                 mg::UnitIdRange( 4 ),
                 mg::UnitIdRange( 1 )
                 },
                 {
                 mg::UnitIdRange( 1 ),
                 mg::UnitIdRange( 4 )
                 } );
     // 1, 2 -> 1-2
     testHelper( {
                 mg::UnitIdRange( 1 ),
                 mg::UnitIdRange( 2 )
                 },
                 {
                 mg::UnitIdRange( 1, 2 )
                 } );
     // 2, 1 -> 1-2
     testHelper( {
                 mg::UnitIdRange( 2 ),
                 mg::UnitIdRange( 1 )
                 },
                 {
                 mg::UnitIdRange( 1, 2 )
                 } );
     // 1, 1 -> 1
     testHelper( {
                 mg::UnitIdRange( 1 ),
                 mg::UnitIdRange( 1 )
                 },
                 {
                 mg::UnitIdRange( 1 )
                 } );
}

TEST( UnitIdRangeTest, CompressValueToRangeTest )
{
     namespace mg = modbus_gateway;
     using UnitIdRangeSet = std::vector< mg::UnitIdRange >;
     auto testHelper = []( UnitIdRangeSet input, const UnitIdRangeSet& expect )
     {
          mg::CompressUnitIdRanges( input );
          ASSERT_EQ( input.size(), expect.size() );
          for( size_t index = 0; index < input.size(); ++index )
          {
               EXPECT_EQ( input[ index ].begin, expect[ index ].begin );
               EXPECT_EQ( input[ index ].end, expect[ index ].end );
          }
     };

     // 1, 1-5 -> 1-5
     testHelper( {
                 mg::UnitIdRange( 1 ),
                 mg::UnitIdRange( 1, 5 )
                 },
                 {
                 mg::UnitIdRange( 1, 5 )
                 } );
     // 1-5, 5 -> 1-5
     testHelper( {
                 mg::UnitIdRange( 1, 5 ),
                 mg::UnitIdRange( 5 )
                 },
                 {
                 mg::UnitIdRange( 1, 5 )
                 } );

     // 1, 2-5 -> 1-5
     testHelper( {
                 mg::UnitIdRange( 1 ),
                 mg::UnitIdRange( 2, 5 )
                 },
                 {
                 mg::UnitIdRange( 1, 5 )
                 } );
     // 1-5, 6 -> 1-6
     testHelper( {
                 mg::UnitIdRange( 1, 5 ),
                 mg::UnitIdRange( 6 )
                 },
                 {
                 mg::UnitIdRange( 1, 6 )
                 } );
     // 1-5, 3 -> 1-5
     testHelper( {
                 mg::UnitIdRange( 1, 5 ),
                 mg::UnitIdRange( 3 )
                 },
                 {
                 mg::UnitIdRange( 1, 5 )
                 } );
     // 1, 3-5 -> 1, 3-5
     testHelper( {
                 mg::UnitIdRange( 1 ),
                 mg::UnitIdRange( 3, 5 )
                 },
                 {
                 mg::UnitIdRange( 1 ),
                 mg::UnitIdRange( 3, 5 )
                 } );

}
TEST( UnitIdRangeTest, CompressRangeToRangeTest )
{
     namespace mg = modbus_gateway;
     using UnitIdRangeSet = std::vector< mg::UnitIdRange >;
     auto testHelper = []( UnitIdRangeSet input, const UnitIdRangeSet& expect )
     {
          mg::CompressUnitIdRanges( input );
          ASSERT_EQ( input.size(), expect.size() );
          for( size_t index = 0; index < input.size(); ++index )
          {
               EXPECT_EQ( input[ index ].begin, expect[ index ].begin );
               EXPECT_EQ( input[ index ].end, expect[ index ].end );
          }
     };

     // 1-2, 4-5 -> 1-2, 4-5
     testHelper( {
                 mg::UnitIdRange( 1, 2 ),
                 mg::UnitIdRange( 4, 5 )
                 },
                 {
                 mg::UnitIdRange( 1, 2 ),
                 mg::UnitIdRange( 4, 5 )
                 } );
     // 1-4, 4-6 -> 1-6
     testHelper( {
                 mg::UnitIdRange( 1, 4 ),
                 mg::UnitIdRange( 4, 6 )
                 },
                 {
                 mg::UnitIdRange( 1, 6 ),
                 } );
     // 1-4, 3-6 -> 1-6
     testHelper( {
                 mg::UnitIdRange( 1, 4 ),
                 mg::UnitIdRange( 3, 6 )
                 },
                 {
                 mg::UnitIdRange( 1, 6 ),
                 } );
     // 1-4, 1-6 -> 1-6
     testHelper( {
                 mg::UnitIdRange( 1, 4 ),
                 mg::UnitIdRange( 1, 6 )
                 },
                 {
                 mg::UnitIdRange( 1, 6 ),
                 } );
     //  1-6, 1-4, -> 1-6
     testHelper( {
                 mg::UnitIdRange( 1, 6 ),
                 mg::UnitIdRange( 1, 4 )
                 },
                 {
                 mg::UnitIdRange( 1, 6 ),
                 } );
     //  2-6, 1-4, -> 1-6
     testHelper( {
                 mg::UnitIdRange( 2, 6 ),
                 mg::UnitIdRange( 1, 4 )
                 },
                 {
                 mg::UnitIdRange( 1, 6 ),
                 } );
}

TEST( UnitIdRangeTest, CompressTest )
{
     namespace mg = modbus_gateway;
     using UnitIdRangeSet = std::vector< mg::UnitIdRange >;
     auto testHelper = []( UnitIdRangeSet input, const UnitIdRangeSet& expect )
     {
          mg::CompressUnitIdRanges( input );
          ASSERT_EQ( input.size(), expect.size() );
          for( size_t index = 0; index < input.size(); ++index )
          {
               EXPECT_EQ( input[ index ].begin, expect[ index ].begin );
               EXPECT_EQ( input[ index ].end, expect[ index ].end );
          }
     };

     testHelper( {
                 mg::UnitIdRange( 1, 10 ),
                 mg::UnitIdRange( 1, 6 ),
                 mg::UnitIdRange( 4, 10 ),
                 mg::UnitIdRange( 5 ),
                 mg::UnitIdRange( 15, 20 ),
                 },
                 {
                 mg::UnitIdRange( 1, 10 ),
                 mg::UnitIdRange( 15, 20 ),
                 } );
}