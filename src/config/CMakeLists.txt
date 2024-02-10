project("mg_config")

set(SOURCE
        config.cpp
        config_service.cpp
        convertors.cpp
        extractor.cpp
        i_transport_config.cpp
        invalid_type_exception.cpp
        invalid_value_exception.cpp
        key_not_found_exception.cpp
        master_config.cpp
        parser_exception.cpp
        rtu_maser_config.cpp
        rtu_slave_config.cpp
        tcp_client_config.cpp
        tcp_server_config.cpp
        trace_deep.cpp
        trace_path.cpp
        unit_id_range.cpp
#        validators.cpp
        value_type.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC
        mg_common
        mg_transport
        modbus
        nlohmann_json
)
