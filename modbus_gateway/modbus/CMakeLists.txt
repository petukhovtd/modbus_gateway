project(modbus)

set(MODBUS_SOURCE
          src/modbus_types.cpp
          src/modbus_framing.cpp
          src/modbus_buffer.cpp
          src/modbus_buffer_tcp_wrapper.cpp
          src/modbus_buffer_ascii_wrapper.cpp
          src/modbus_buffer_rtu_wrapper.cpp
          src/modbus_buffer_wrapper.cpp
)

add_library(${PROJECT_NAME} STATIC ${MODBUS_SOURCE})

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_compile_options(${PROJECT_NAME} PRIVATE
          -Wall
          -Werror
          -Wextra
          -Wpedantic
          -Weffc++
          -Wold-style-cast
          -Wctor-dtor-privacy
          -Woverloaded-virtual
          -Wno-sign-conversion
          -Wno-unused-parameter
          -Wno-uninitialized
)
