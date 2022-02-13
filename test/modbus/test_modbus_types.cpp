#include <gtest/gtest.h>
#include <modbus/modbus_types.h>

using namespace modbus;

TEST( ModbusTypes, BufferSize )
{
     EXPECT_EQ( MakeAduBuffer().size(), aduBufferMaxSize );
}

TEST( ModbusTypes, GetAduStart )
{
     EXPECT_EQ( GetAduStart( FrameType::RTU ), aduRtuStart );
     EXPECT_EQ( GetAduStart( FrameType::TCP ), aduTcpStart );
     EXPECT_EQ( GetAduStart( FrameType::ASCII ), aduAsciiStart );
}

TEST( ModbusTypes, GetAduMaxSize )
{
     EXPECT_EQ( GetAduMaxSize( FrameType::RTU ), aduRtuMaxSize );
     EXPECT_EQ( GetAduMaxSize( FrameType::TCP ), aduTcpMaxSize );
     EXPECT_EQ( GetAduMaxSize( FrameType::ASCII ), aduAsciiMaxSize );
}

TEST( ModbusTypes, GetPosForTypeCommon )
{
     EXPECT_EQ( GetPosForType( FrameType::RTU, CommonField::UnitId ), unitIdGeneralPos );
     EXPECT_EQ( GetPosForType( FrameType::RTU, CommonField::FunctionCode ), functionCodeGeneralPos );
     EXPECT_EQ( GetPosForType( FrameType::RTU, CommonField::DataStart ), dataGeneralPos );

     EXPECT_EQ( GetPosForType( FrameType::TCP, CommonField::UnitId ), unitIdGeneralPos );
     EXPECT_EQ( GetPosForType( FrameType::TCP, CommonField::FunctionCode ), functionCodeGeneralPos );
     EXPECT_EQ( GetPosForType( FrameType::TCP, CommonField::DataStart ), dataGeneralPos );

     EXPECT_EQ( GetPosForType( FrameType::ASCII, CommonField::UnitId ), unitIdGeneralPos );
     EXPECT_EQ( GetPosForType( FrameType::ASCII, CommonField::FunctionCode ), functionCodeASCIIPos );
     EXPECT_EQ( GetPosForType( FrameType::ASCII, CommonField::DataStart ), dataASCIIPos );
}
