#pragma once

namespace modbus_gateway {

enum class TransportType {
  TcpServer,
  RtuSlave,
  TcpClient,
  RtuMaster
};

}
