set(TEST_TARGET test)

set(SOURCES_TEST
          modbus_common.cpp

          modbus/test_modbus_types.cpp
          modbus/test_modbus_framing.cpp
          modbus/test_modbus_buffer.cpp
          modbus/test_modbus_buffer_tcp_wrapper.cpp
          modbus/test_modbus_buffer_ascii_wrapper.cpp
          modbus/test_modbus_buffer_rtu_wrapper.cpp

          single_router.cpp
          modbus_message_actor.cpp
          modbus_tcp_client.cpp

          modbus_gateway/test_modbus_tcp_connection.cpp
          modbus_gateway/test_config_convertors.cpp
          modbus_gateway/test_config_extractor.cpp
          modbus_gateway/test_config.cpp
          modbus_gateway/test_unit_id_range.cpp
)

add_executable(${TEST_TARGET} ${SOURCES_TEST})

target_link_libraries(
          ${TEST_TARGET}
          gtest
          gtest_main
          ${LIBS}
)

