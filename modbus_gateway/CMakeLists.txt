include_directories(modbus/include)

set(MODBUS_GATEWAY_SOURCE
          common/fmt_logger.cpp
          main.cpp
          tcp_server.cpp
          modbus_tcp_slave.cpp
          messages/client_disconnect_message.cpp
          messages/modbus_message.cpp
)

add_executable(${PROJECT_NAME} ${MODBUS_GATEWAY_SOURCE})

target_include_directories(${PROJECT_NAME} PRIVATE
          .
)

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

target_link_libraries(
          ${PROJECT_NAME}
          pthread
          exchange
          spdlog
          modbus
)

add_subdirectory(modbus)
add_subdirectory(test)
