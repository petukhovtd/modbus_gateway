include_directories(modbus/include)

set(MODBUS_GATEWAY_SOURCE
          common/fmt_logger.cpp

          messages/client_disconnect_message.cpp
          messages/modbus_message.cpp

          modbus_message_info.cpp

          modbus_tcp_server.cpp
          modbus_tcp_connection.cpp
          modbus_tcp_master.cpp
)

add_executable(${PROJECT_NAME} ${MODBUS_GATEWAY_SOURCE} main.cpp)
add_library(lib${PROJECT_NAME} STATIC ${MODBUS_GATEWAY_SOURCE})

include_directories(${PROJECT_NAME} PRIVATE .)

target_compile_options(${PROJECT_NAME} PRIVATE
          -Wall
          -Werror
          -Wextra
          -Wpedantic
          -Woverloaded-virtual
          -Wno-sign-conversion
          -Wno-unused-parameter
          -Wno-uninitialized
)

set(LIBS pthread exchange spdlog modbus lib${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} ${LIBS})

add_subdirectory(modbus)
add_subdirectory(test)
