
add_compile_definitions(ASIO_NO_DEPRECATED)
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE asio/asio/include)

add_subdirectory(cxxopts)
add_subdirectory(exchange)
if(MG_BUILD_TEST)
    add_subdirectory(googletest)
endif ()
add_subdirectory(json)
add_subdirectory(modbus)
add_subdirectory(spdlog)
