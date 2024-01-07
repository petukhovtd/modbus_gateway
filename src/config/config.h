#pragma once

#include <config/config_service.h>
#include <config/i_transport_config.h>

#include <istream>
#include <vector>

namespace modbus_gateway {

class ConfigService;

struct Config {
public:
  explicit Config(std::istream &in);

  void Validate() const;

  ConfigService configService;
  std::vector<TransportConfigPtr> slaves;
  std::vector<TransportConfigPtr> masters;
};

}// namespace modbus_gateway
