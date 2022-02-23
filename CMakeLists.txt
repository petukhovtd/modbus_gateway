cmake_minimum_required(VERSION 3.13)
project(modbus_gateway)

include_directories(
          contrib/asio/asio/include
          contrib/exchange/include
          contrib/googletest/googletest/include
          contrib/spdlog/include
)

add_subdirectory(contrib/exchange)
add_subdirectory(contrib/googletest)
add_subdirectory(contrib/spdlog)
add_subdirectory(modbus_gateway)
