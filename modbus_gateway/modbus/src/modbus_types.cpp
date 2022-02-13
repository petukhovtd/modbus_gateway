#include "modbus/modbus_types.h"

#include <stdexcept>
#include <array>

namespace modbus
{

AduBuffer MakeAduBuffer()
{
     return AduBuffer( aduBufferMaxSize, aduDefaultValue );
}

size_t GetAduStart( FrameType type )
{
     switch( type )
     {
          case RTU: return aduRtuStart;
          case ASCII: return aduAsciiStart;
          case TCP: return aduTcpStart;
          default:
               throw std::logic_error( "unknown frame type" );
     }
}

size_t GetAduMaxSize( FrameType type )
{
     switch( type )
     {
          case RTU: return aduRtuMaxSize;
          case ASCII: return aduAsciiMaxSize;
          case TCP: return aduTcpMaxSize;
          default:
               throw std::logic_error( "unknown frame type" );
     }
}

std::string GetTypeName( FrameType type )
{
     switch( type )
     {
          case RTU: return "RTU";
          case ASCII: return "ASCII";
          case TCP: return "TCP";
          default:
               throw std::logic_error( "unknown frame type" );
     }
}

size_t GetPosForType( FrameType frameType, CommonField commonField )
{
     switch( commonField )
     {
          case CommonField::UnitId:
               return unitIdGeneralPos;
          case CommonField::FunctionCode:
               switch( frameType )
               {
                    case RTU:
                    case TCP:
                         return functionCodeGeneralPos;
                    case ASCII:
                         return functionCodeASCIIPos;
                    default:
                         throw std::logic_error( "unknown frame type" );
               }
          case CommonField::DataStart:
               switch( frameType )
               {
                    case RTU:
                    case TCP:
                         return dataGeneralPos;
                    case ASCII:
                         return dataASCIIPos;
                    default:
                         throw std::logic_error( "unknown frame type" );
               }
          default:
               throw std::logic_error( "unknown common field type" );
     }
}

size_t GetPosForType( TcpField tcpField )
{
     switch( tcpField )
     {
          case TcpField::TransactionId:
               return transactionIdPos;
          case TcpField::ProtocolId:
               return protocolIdPos;
          case TcpField::Length:
               return lengthPos;
          default:
               throw std::logic_error( "unknown tcp field type" );
     }
}

size_t CalculatePduSize( FrameType type, size_t aduSize )
{
     switch( type )
     {
          case RTU:
               return aduSize - rtuOverhead;
          case ASCII:
               return aduSize - asciiOverhead;
          case TCP:
               return aduSize - mbapSize;
          default:
               throw std::logic_error( "unknown frame type" );
     }
}

size_t CalculateAduSize( FrameType type, size_t pduSize )
{
     switch( type )
     {
          case RTU:
               return pduSize + rtuOverhead;
          case ASCII:
               return pduSize + asciiOverhead;
          case TCP:
               return pduSize + mbapSize;
          default:
               throw std::logic_error( "unknown frame type" );
     }
}

}
