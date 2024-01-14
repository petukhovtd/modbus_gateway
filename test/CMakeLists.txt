project(test)

set(SOURCE
        common/context_runner.cpp
        common/misc.cpp
        common/modbus_message_actor.cpp
        common/single_router.cpp
        common/test_modbus_tcp_client.cpp
        common/modbus_message_sender.cpp
        common/test_modbus_tcp_server.cpp
        common/test_modbus_rtu_master.cpp
        common/rtu_creator.cpp
        common/test_modbus_rtu_slave.cpp

        test_main.cpp

        test_modbus_tcp_connection.cpp
        test_modbus_tcp_client.cpp
        test_modbus_rtu_slave.cpp
        test_modbus_rtu_master.cpp
        test_config.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE})
target_include_directories(${PROJECT_NAME} PRIVATE .)
target_link_libraries(${PROJECT_NAME} PRIVATE
        gtest
        mg
)

