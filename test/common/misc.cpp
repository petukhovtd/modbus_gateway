#include <common/misc.h>

#include <exchange/actor_storage_table.h>
#include <exchange/id_generator_reuse.h>
#include <exchange/exchange.h>

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

exchange::ExchangePtr MakeExchange() {
  return std::make_shared<exchange::Exchange>(std::make_unique<exchange::ActorStorageTable>(),
                                              std::make_shared<exchange::IdGeneratorReuse>());
}

}// namespace test
