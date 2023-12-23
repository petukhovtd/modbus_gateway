#ifndef MODBUS_GATEWAY_VALIDATORS_H
#define MODBUS_GATEWAY_VALIDATORS_H

#include <config/i_transport_config.h>
#include <config/trace_path.h>

#include <vector>

namespace modbus_gateway
{

void ValidateServers( TracePath& tracePath, const std::vector< TransportConfigPtr >& servers );

void ValidateClients( TracePath& tracePath, const std::vector< TransportConfigPtr >& clients );

}


#endif //MODBUS_GATEWAY_VALIDATORS_H
