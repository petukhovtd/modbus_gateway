cmake_minimum_required(VERSION 3.13)
project(modbus_gateway)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(ASIO_NO_DEPRECATED)

include_directories(
          contrib/asio/asio/include
          contrib/exchange/include
          contrib/googletest/googletest/include
          contrib/spdlog/include
          contrib/json/include
          contrib/cxxopts/include
)

add_subdirectory(contrib/exchange)
add_subdirectory(contrib/googletest)
add_subdirectory(contrib/spdlog)
add_subdirectory(contrib/json)
add_subdirectory(contrib/cxxopts)
add_subdirectory(modbus_gateway)
