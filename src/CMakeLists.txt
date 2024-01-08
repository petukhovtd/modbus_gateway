set(SOURCE
        common/fmt_logger.cpp

        message/modbus_message_info.cpp
        message/modbus_message.cpp
        message/client_disconnect_message.cpp

        transport/router.cpp
        transport/modbus_tcp_connection.cpp
        transport/modbus_tcp_server.cpp
        transport/modbus_tcp_client.cpp
        transport/modbus_rtu_slave.cpp
        transport/modbus_rtu_master.cpp

        config/trace_path.cpp
        config/trace_deep.cpp
        config/value_type.cpp
        config/parser_exception.cpp
        config/key_not_found_exception.cpp
        config/invalid_type_exception.cpp
        config/invalid_value_exception.cpp
        config/convertors.cpp
        config/unit_id_range.cpp
        config/extractor.cpp
        config/config_service.cpp
        config/i_transport_config.cpp
        config/tcp_server_config.cpp
        config/rtu_slave_config.cpp
        config/tcp_client_config.cpp
        config/rtu_maser_config.cpp
        config/config.cpp

        command_line/command_line.cpp
        command_line/command_line_exception.cpp
)

add_library(mbgw STATIC ${SOURCE})
target_include_directories(mbgw PUBLIC .)
target_link_libraries(mbgw PUBLIC
        spdlog
        modbus
        exchange
        asio
        nlohmann_json
        cxxopts
)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE
        mbgw
)
