cmake_minimum_required(VERSION 3.13)
project(modbus_gateway)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

add_subdirectory(contrib)
#add_subdirectory(modbus_gateway)

set(SOURCE
        src/common/fmt_logger.cpp
        src/main.cpp
)

message(${SOURCE})

add_executable(${PROJECT_NAME} ${SOURCE})

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE
    spdlog
)
