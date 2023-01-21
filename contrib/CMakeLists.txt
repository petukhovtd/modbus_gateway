
add_compile_definitions(ASIO_NO_DEPRECATED)
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE asio/asio/include)

add_subdirectory(exchange)
add_subdirectory(googletest)
add_subdirectory(spdlog)
add_subdirectory(json)
add_subdirectory(cxxopts)
