project("mg_message")

set(SOURCE
        client_disconnect_message.cpp
        modbus_message_info.cpp
        modbus_message.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC
        mg_common
        exchange
        modbus
)
