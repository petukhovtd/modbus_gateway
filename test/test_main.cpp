#include <gtest/gtest.h>
#include <common/logger.h>


int main(int argc, char *argv[]) {

    modbus_gateway::Logger::SetLogLevel(modbus_gateway::Logger::LogLevel::Trace);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();;
}
