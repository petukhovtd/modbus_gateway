#include "unit_id_range.h"
#include "config_types.h"
#include "extractor.h"

namespace modbus_gateway
{

UnitIdRange::UnitIdRange( TracePath& tracePath, const nlohmann::json::value_type& obj )
{
     const NumericRangeType type = ExtractNumericRangeType( tracePath, obj );
     switch( type )
     {
          case NumericRangeType::Range:
               begin = ExtractModbusUnitId( tracePath, obj, "begin" );
               end = ExtractModbusUnitId( tracePath, obj, "end" );
               Normalize();
               break;
          case NumericRangeType::Value:
               begin = end = ExtractModbusUnitId( tracePath, obj, "value" );
               break;
          default:
               throw std::logic_error( "unsupported range type" );
     }
}

void UnitIdRange::Normalize()
{
     if( begin > end )
     {
          std::swap( begin, end );
     }
}

UnitIdRange::UnitIdRange( modbus::UnitId b, modbus::UnitId e )
: begin( b )
, end( e )
{
     Normalize();
}

UnitIdRange::UnitIdRange( modbus::UnitId value )
: begin( value )
, end( value )
{
}

void CompressUnitIdRanges( std::vector< UnitIdRange >& unitIds )
{
     if( unitIds.size() <= 1 )
     {
          return;
     }

     std::sort( unitIds.begin(), unitIds.end(), []( const UnitIdRange& lhs, const UnitIdRange& rhs )
     {
          return std::tie( lhs.begin, lhs.end ) < std::tie( rhs.begin, rhs.end );
     });

     for( auto it = unitIds.begin(); it != std::prev( unitIds.end() ); )
     {
          auto next = std::next( it );
          if( it->end >= next->begin || ( it->end + 1 ) == next->begin )
          {
               it->end = std::max( it->end, next->end );
               unitIds.erase( next );
          }
          else
          {
               ++it;
          }
     }
}

}
