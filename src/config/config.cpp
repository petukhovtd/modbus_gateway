#include <config/config.h>
#include <config/extractor.h>
#include <config/trace_path.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace modbus_gateway {

Config::Config(std::istream &in)
{
  auto data = json::parse(in);

  TracePath tracePath;
  tracePath.Push("config");

  configService = ExtractConfigService(tracePath, data);
  slaves = ExtractSlaves(tracePath, data);
  masters = ExtractMasters(tracePath, data);
}

void Config::Validate() const {
  TracePath tracePath;
  tracePath.Push("config");

  //     ValidateServers( tracePath, servers_ );
  //     ValidateClients( tracePath, clients_ );
}

}// namespace modbus_gateway
