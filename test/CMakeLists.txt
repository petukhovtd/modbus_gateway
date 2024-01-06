project(test)

set(SOURCE
        common/context_runner.cpp
        common/misc.cpp
        common/modbus_message_actor.cpp
        common/single_router.cpp
        common/test_modbus_tcp_client.cpp

        test_main.cpp

        test_modbus_tcp_connection.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE})
target_include_directories(${PROJECT_NAME} PRIVATE .)
target_link_libraries(${PROJECT_NAME} PRIVATE
        gtest
        modbus
        exchange
        spdlog
        mbgw
)

