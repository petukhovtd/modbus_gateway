cmake_minimum_required(VERSION 3.13)
project(modbus_gateway)

set(CMAKE_CXX_STANDARD 17)

add_compile_options( -Wall )
add_compile_options( -Werror )
add_compile_options( -Wextra )
add_compile_options( -Wpedantic )

include_directories(
          ${PROJECT_SOURCE_DIR}/modbus_gateway
          ${PROJECT_SOURCE_DIR}/contrib/exchange/include
          ${PROJECT_SOURCE_DIR}/contrib/asio/asio/include
          ${PROJECT_SOURCE_DIR}/contrib/spdlog/include
          ${PROJECT_SOURCE_DIR}/modbus_gateway/modbus/include
)

set(SOURCE
          ${PROJECT_SOURCE_DIR}/modbus_gateway/common/fmt_logger.cpp
          ${PROJECT_SOURCE_DIR}/modbus_gateway/main.cpp
          ${PROJECT_SOURCE_DIR}/modbus_gateway/tcp_server.cpp
          ${PROJECT_SOURCE_DIR}/modbus_gateway/modbus_tcp_slave.cpp
          ${PROJECT_SOURCE_DIR}/modbus_gateway/messages/client_disconnect_message.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE})

set(LIBS exchange pthread spdlog modbus)

target_link_libraries(
          ${PROJECT_NAME}
          ${LIBS}
)

add_subdirectory(contrib/exchange)
add_subdirectory(contrib/googletest)
add_subdirectory(contrib/spdlog)
add_subdirectory(modbus_gateway/modbus)

add_subdirectory(test)