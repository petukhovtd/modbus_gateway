#include <gtest/gtest.h>
#include <common/logger.h>
#include <common/rtu_creator.h>

int main(int argc, char *argv[]) {

    modbus_gateway::Logger::SetLogLevel(modbus_gateway::Logger::LogLevel::Trace);

    test::RtuCreator rtuCreator;
//    rtuCreator.Run();

    testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

//    rtuCreator.Stop();

    return result;
}
