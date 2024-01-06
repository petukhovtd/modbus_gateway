#include <common/misc.h>

#include <stdexcept>

namespace test {

modbus::ModbusBuffer MakeModbusBuffer(const modbus::AduBuffer &frame, modbus::FrameType type) {
  modbus::ModbusBuffer modbusBuffer(type);
  std::copy(frame.begin(), frame.end(), modbusBuffer.begin());
  if (!modbusBuffer.SetAduSize(frame.size())) {
    throw std::logic_error("invalid adu size");
  }
  return modbusBuffer;
}

}// namespace test
