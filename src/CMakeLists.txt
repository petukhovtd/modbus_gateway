set(SOURCE
        common/fmt_logger.cpp

        message/modbus_message_info.cpp
        message/modbus_message.cpp
        message/client_disconnect_message.cpp

        transport/router.cpp
        transport/modbus_tcp_connection.cpp
        transport/modbus_tcp_server.cpp
        transport/modbus_tcp_client.cpp
        transport/modbus_rtu_slave.cpp
        transport/modbus_rtu_master.cpp
)

add_library(mbgw STATIC ${SOURCE})
target_include_directories(mbgw PUBLIC .)
target_link_libraries(mbgw PUBLIC
        spdlog
        modbus
        exchange
        asio
)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE
        mbgw
)
