project("mg_command_line")

set(SOURCE
        command_line.cpp
        command_line_exception.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC
        cxxopts
)
