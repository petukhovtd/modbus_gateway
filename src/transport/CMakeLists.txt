project("mg_transport")

set(SOURCE
        i_modbus_slave.cpp
        modbus_rtu_master.cpp
        modbus_rtu_slave.cpp
        modbus_tcp_client.cpp
        modbus_tcp_connection.cpp
        modbus_tcp_server.cpp
        router.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC
        mg_common
        mg_message
        exchange
        modbus
)
