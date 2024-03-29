set(MG_CXX_OPT
        -Wall
#        -Werror
        -Wextra
        -Wpedantic
        -Weffc++
        -Wold-style-cast
        -Wconversion
        -Wctor-dtor-privacy
        -Woverloaded-virtual
        -Wno-sign-conversion
        -Wno-unused-parameter
        -Wno-uninitialized
)

add_subdirectory(command_line)
add_subdirectory(common)
add_subdirectory(message)
add_subdirectory(transport)
add_subdirectory(config)

set(SOURCE
        modbus_gateway.cpp
)

add_library(mg STATIC ${SOURCE})
target_compile_options(mg PRIVATE ${MG_CXX_OPT})
target_include_directories(mg PUBLIC .)
target_link_libraries(mg PUBLIC
        mg_command_line
        mg_common
        mg_message
        mg_transport
        mg_config
)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE
        mg
)
if(MG_BUILD_STATIC)
    target_link_libraries(${PROJECT_NAME} PRIVATE -static)
endif ()
