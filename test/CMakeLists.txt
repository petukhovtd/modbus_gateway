enable_testing()

set(TEST_TARGET test)

include_directories(
          .
)

set(SOURCES_TEST
          modbus/test_modbus_types.cpp
          modbus/test_modbus_framing.cpp
          modbus/test_modbus_buffer.cpp
          modbus/test_modbus_buffer_tcp_wrapper.cpp
          )

add_executable(${TEST_TARGET} ${SOURCES_TEST})

target_link_libraries(
          ${TEST_TARGET}
          ${LIBS}
          gtest
          gtest_main
)