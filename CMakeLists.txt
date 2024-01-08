cmake_minimum_required(VERSION 3.13)
project(modbus_gateway)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

add_subdirectory(contrib)

add_subdirectory(src)
add_subdirectory(test)
