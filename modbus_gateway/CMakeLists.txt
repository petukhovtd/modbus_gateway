include_directories(modbus/include)

set(MODBUS_GATEWAY_SOURCE
          common/fmt_logger.cpp

          config/value_type.cpp
          config/trace_path.cpp
          config/trace_deep.cpp
          config/parser_exception.cpp
          config/key_not_found_exception.cpp
          config/invalid_value_exception.cpp
          config/invalid_type_exception.cpp
          config/extractor.cpp
          config/convertors.cpp
          config/i_transport_config.cpp
          config/tcp_server_config.cpp
          config/unit_id_range.cpp
          config/modbus_client_config.cpp
          config/tcp_client_config.cpp
          config/config.cpp
          config/validators.cpp

          messages/client_disconnect_message.cpp
          messages/modbus_message.cpp

          modbus_message_info.cpp

          modbus_tcp_server.cpp
          modbus_tcp_connection.cpp
          modbus_tcp_master.cpp
)

add_library(lib${PROJECT_NAME} STATIC ${MODBUS_GATEWAY_SOURCE})

add_executable(${PROJECT_NAME} main.cpp)

include_directories(${PROJECT_NAME} PRIVATE .)

target_compile_options(${PROJECT_NAME} PRIVATE
          -Wall
          -Werror
          -Wextra
          -Wpedantic
          -Woverloaded-virtual
          -Wno-sign-conversion
          -Wno-unused-parameter
          -Wno-uninitialized
)

set(LIBS pthread exchange spdlog modbus lib${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} ${LIBS})

add_subdirectory(modbus)
add_subdirectory(test)
