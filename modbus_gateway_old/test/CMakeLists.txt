project(unit-test)

set(SOURCES_TEST
          modbus_common.cpp

          single_router.cpp
          modbus_message_actor.cpp
          modbus_tcp_client.cpp

          modbus_gateway/test_modbus_tcp_connection.cpp
          modbus_gateway/test_config_convertors.cpp
          modbus_gateway/test_config_extractor.cpp
          modbus_gateway/test_config.cpp
          modbus_gateway/test_unit_id_range.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES_TEST})
target_link_libraries(${PROJECT_NAME} PRIVATE
          gtest
          gtest_main
          libmodbus_gateway
)

