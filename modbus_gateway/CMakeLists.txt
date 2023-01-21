set(${PROJECT_NAME}_LIBS pthread exchange spdlog modbus asio nlohmann_json cxxopts)
set(${PROJECT_NAME}_SOURCE
          common/fmt_logger.cpp

          config/config.cpp
          config/convertors.cpp
          config/extractor.cpp
          config/i_transport_config.cpp
          config/invalid_type_exception.cpp
          config/invalid_value_exception.cpp
          config/key_not_found_exception.cpp
          config/modbus_client_config.cpp
          config/parser_exception.cpp
          config/tcp_client_config.cpp
          config/tcp_server_config.cpp
          config/trace_deep.cpp
          config/trace_path.cpp
          config/unit_id_range.cpp
          config/validators.cpp
          config/value_type.cpp

          messages/client_disconnect_message.cpp
          messages/modbus_message.cpp

          command_line.cpp
          command_line_exception.cpp
          i_modbus_slave.cpp
          modbus_message_info.cpp
          modbus_tcp_connection.cpp
          modbus_tcp_master.cpp
          modbus_tcp_server.cpp
          router.cpp

          modbus_gateway.cpp
          )

add_subdirectory(modbus)
add_subdirectory(test)


add_library(lib${PROJECT_NAME} ${${PROJECT_NAME}_SOURCE})
target_include_directories(lib${PROJECT_NAME} PUBLIC .)
target_link_libraries(lib${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_LIBS})

add_executable(${PROJECT_NAME} main.cpp ${${PROJECT_NAME}_SOURCE})
target_include_directories(${PROJECT_NAME} PRIVATE .)
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
target_link_libraries(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_LIBS})
