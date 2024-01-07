#include <config/config.h>
#include <config/extractor.h>
#include <config/trace_path.h>

#include <config/keys.h>
#include <config/rtu_maser_config.h>
#include <config/tcp_client_config.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace modbus_gateway {

Config::Config(std::istream &in) {
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

  {
    std::vector<size_t> defaultIdIndexes;
    for (size_t masterIndex = 0; masterIndex < masters.size(); ++masterIndex) {
      const auto &master = masters[masterIndex];
      switch (master->GetType()) {

      case ITransportConfig::Type::TcpClient: {
        auto ptr = std::dynamic_pointer_cast<modbus_gateway::TcpClientConfig>(master);
        if (ptr && ptr->unitIdSet.empty()) {
          defaultIdIndexes.push_back(masterIndex);
        }
      } break;
      case ITransportConfig::Type::RtuMaster: {
        auto ptr = std::dynamic_pointer_cast<modbus_gateway::RtuMasterConfig>(master);
        if (ptr && ptr->unitIdSet.empty()) {
          defaultIdIndexes.push_back(masterIndex);
        }
      } break;
      case ITransportConfig::Type::TcpServer: break;
      case ITransportConfig::Type::RtuSlave: break;
      default:
        break;
      }
    }

    if (defaultIdIndexes.size() > 1) {
      TraceDeep td(tracePath, keys::masters);
      throw InvalidValueException(td, "more then one master with default unit id");
    }
  }

  //     ValidateServers( tracePath, servers_ );
  //     ValidateClients( tracePath, clients_ );
}

}// namespace modbus_gateway
