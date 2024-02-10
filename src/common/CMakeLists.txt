project("mg_common")

set(SOURCE
        fmt_logger.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC
        spdlog
        asio
        modbus
)
