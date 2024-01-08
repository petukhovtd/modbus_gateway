#pragma once

#include <modbus/modbus_buffer.h>

#include <memory>

namespace modbus_gateway {

using ModbusBufferPtr = std::shared_ptr<modbus::ModbusBuffer>;

}
