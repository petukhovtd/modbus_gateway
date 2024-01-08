cmake_minimum_required(VERSION 3.5)

project(modbus_gateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

option(MG_BUILD_TEST "Build unit test" OFF)
option(MG_BUILD_STATIC "Build static executable" OFF)

add_subdirectory(contrib)

add_subdirectory(src)
if (MG_BUILD_TEST)
    add_subdirectory(test)
endif ()
