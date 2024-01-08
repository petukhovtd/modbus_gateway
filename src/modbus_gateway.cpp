#include <modbus_gateway.h>

#include <common/types_asio.h>

#include <transport/i_modbus_slave.h>
#include <transport/modbus_rtu_master.h>
#include <transport/modbus_rtu_slave.h>
#include <transport/modbus_tcp_client.h>
#include <transport/modbus_tcp_server.h>
#include <transport/router.h>

#include <config/master_config.h>
#include <config/rtu_maser_config.h>
#include <config/rtu_slave_config.h>
#include <config/tcp_client_config.h>
#include <config/tcp_server_config.h>

#include <exchange/actor_storage_ht.h>
#include <exchange/exchange.h>
#include <exchange/iactor.h>

namespace modbus_gateway {

struct Master {
  TransportConfigPtr config;
  exchange::ActorPtr actor;
};

struct Slave {
  TransportConfigPtr config;
  ModbusSlavePtr Slave;
};

std::vector<Master> MakeMasters(const std::vector<TransportConfigPtr> &mastersConfig, const exchange::ExchangePtr &exchange, const ContextPtr &context) {

  std::vector<Master> masters;
  masters.reserve(mastersConfig.size());

  for (const auto &masterConfig : mastersConfig) {
    switch (masterConfig->GetType()) {
    case TransportType::TcpClient: {
      const auto &tcpClientConfig = std::dynamic_pointer_cast<TcpClientConfig>(masterConfig);
      if (!tcpClientConfig) {
        throw std::logic_error("BUG! cast to TcpClientConfig failed");
      }
      auto tcpClient = ModbusTcpClient::Create(exchange,
                                               context,
                                               tcpClientConfig->address,
                                               tcpClientConfig->port,
                                               tcpClientConfig->timeout);
      const auto actorId = exchange->Add(tcpClient);
      MG_DEBUG("MG::MakeMasters: Create modbus tcp client; address {}, port {}, timeout {}, actor id {}",
               tcpClientConfig->address.to_string(), tcpClientConfig->port, tcpClientConfig->timeout.count(), actorId);

      Master client = {tcpClientConfig, tcpClient};
      if (tcpClientConfig->unitIdSet.empty()) {
        masters.insert(masters.begin(), client);
      } else {
        masters.push_back(client);
      }
    } break;
    case TransportType::RtuMaster: {
      const auto &rtuMasterConfig = std::dynamic_pointer_cast<RtuMasterConfig>(masterConfig);
      if (!rtuMasterConfig) {
        throw std::logic_error("BUG! cast to TcpClientConfig failed");
      }
      auto rtuMaster = ModbusRtuMaster::Create(exchange,
                                               context,
                                               rtuMasterConfig->device,
                                               rtuMasterConfig->rtuOptions,
                                               rtuMasterConfig->timeout,
                                               rtuMasterConfig->GetFrameType());
      const auto actorId = exchange->Add(rtuMaster);
      MG_DEBUG("MG::MakeMasters: create modbus rtu master; device {}, timeout {}, frame type {} actor id {}",
               rtuMasterConfig->device, rtuMasterConfig->timeout.count(), rtuMasterConfig->GetFrameType(), actorId);

      Master client = {rtuMasterConfig, rtuMaster};
      if (rtuMasterConfig->unitIdSet.empty()) {
        masters.insert(masters.begin(), client);
      } else {
        masters.push_back(client);
      }
    } break;
    default:
      throw std::logic_error("BUG! unknown client type");
    }
  }

  return masters;
}

RouterPtr MakeRouter(const std::vector<Master> &masters) {
  std::shared_ptr<Router> router = nullptr;

  if (masters.empty()) {
    return router;
  }

  auto masterIt = masters.begin();

  {
    const auto &masterConfig = std::dynamic_pointer_cast<MasterConfig>(masterIt->config);
    if (!masterConfig) {
      throw std::logic_error("BUG! cast to MasterConfig failed");
    }

    // we know that first is default or haven't default
    if (masterConfig->unitIdSet.empty()) {
      const auto id = masterIt->actor->GetId();
      router = std::make_shared<Router>(id);
      MG_DEBUG("MG::MakeMasters: route by default to actor id {}", id);
      ++masterIt;
    } else {
      router = std::make_shared<Router>();
    }
  }

  std::for_each(masterIt, masters.end(), [router](const Master &master) {
    const auto &clientConfig = std::dynamic_pointer_cast<MasterConfig>(master.config);
    if (!clientConfig) {
      throw std::logic_error("BUG! cast to MasterConfig failed");
    }

    for (const auto &unitIdRange : clientConfig->unitIdSet) {
      for (modbus::UnitId unitId = unitIdRange.begin; unitId <= unitIdRange.end; ++unitId) {
        const auto id = master.actor->GetId();
        router->Set(unitId, id);
        MG_DEBUG("MG::MakeMasters: route unit id {} to actor id {}", unitId, id)
      }
    }
  });

  return router;
}

std::vector<Slave> MakeSlaves(const std::vector<TransportConfigPtr> &slavesConfigs, const exchange::ExchangePtr &exchange, const ContextPtr &context, const RouterPtr &router) {
  std::vector<Slave> slaves;
  slaves.reserve(slavesConfigs.size());

  for (const auto &slavesConfig : slavesConfigs) {
    switch (slavesConfig->GetType()) {
    case TransportType::TcpServer: {
      const auto &tcpServerConfig = std::dynamic_pointer_cast<TcpServerConfig>(slavesConfig);
      if (!tcpServerConfig) {
        throw std::logic_error("BUG! cast to TcpServerConfig failed");
      }
      std::shared_ptr<ModbusTcpServer> tcpServer = ModbusTcpServer::Create(exchange,
                                                                           context,
                                                                           tcpServerConfig->address,
                                                                           tcpServerConfig->port,
                                                                           router);
      const exchange::ActorId id = exchange->Add(tcpServer);
      MG_DEBUG("MG::MakeMasters: create modbus tcp server address {}, port {}, actor id {}",
               tcpServerConfig->address.to_string(), tcpServerConfig->port, id)
      Slave server = {tcpServerConfig, tcpServer};
      slaves.push_back(server);
    } break;
    case TransportType::RtuSlave: {
      const auto &rtuSlaveConfig = std::dynamic_pointer_cast<RtuSlaveConfig>(slavesConfig);
      if (!rtuSlaveConfig) {
        throw std::logic_error("BUG! cast to TcpServerConfig failed");
      }
      std::shared_ptr<ModbusRtuSlave> rtuSlave = ModbusRtuSlave::Create(exchange,
                                                                        context,
                                                                        rtuSlaveConfig->device,
                                                                        rtuSlaveConfig->rtuOptions,
                                                                        router,
                                                                        rtuSlaveConfig->GetFrameType());
      const exchange::ActorId id = exchange->Add(rtuSlave);
      MG_DEBUG("MG::MakeMasters: create modbus rtu slave device {}, frame type {}, actor id {}",
               rtuSlaveConfig->device, rtuSlaveConfig->GetFrameType(), id)
      Slave server = {rtuSlaveConfig, rtuSlave};
      slaves.push_back(server);
    } break;
    default:
      throw std::logic_error("BUG! unknown client type");
    }
  }

  return slaves;
}

int ModbusGateway(const Config &config) {
  auto actorStorage = std::make_unique<exchange::ActorStorageHT>();
  auto exchange = std::make_shared<exchange::Exchange>(std::move(actorStorage));

  ContextPtr context = std::make_shared<ContextPtr::element_type>(config.configService.threads);
  auto work = asio::executor_work_guard(context->get_executor());

  asio::signal_set signalSet(*context);
  signalSet.add(SIGINT);
  signalSet.add(SIGILL);
  signalSet.add(SIGABRT);
  signalSet.add(SIGFPE);
  signalSet.add(SIGSEGV);
  signalSet.add(SIGTERM);

  signalSet.async_wait([context](const asio::error_code &ec, int signalNumber) {
    if (ec) {
      MG_INFO("SignalSet::wait: error: {}", ec.message());
      return;
    }
    MG_INFO("SignalSet::wait: signal {}", signalNumber)
    context->stop();
  });

  std::vector<Master> masters = MakeMasters(config.masters, exchange, context);
  if (masters.empty()) {
    throw std::logic_error("BUG! masters is empty");
  }

  RouterPtr router = MakeRouter(masters);
  if (!router) {
    throw std::logic_error("BUG! router is null");
  }

  std::vector<Slave> slaves = MakeSlaves(config.slaves, exchange, context, router);
  if (slaves.empty()) {
    throw std::logic_error("BUG! slaves is empty");
  }

  MG_INFO("start slaves");
  for (auto &slave : slaves) {
    slave.Slave->Start();
  }

  MG_INFO("starting")
  context->run();
  MG_INFO("stopping")

  return EXIT_SUCCESS;
}

}// namespace modbus_gateway