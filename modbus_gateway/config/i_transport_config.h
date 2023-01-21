#ifndef MODBUS_GATEWAY_I_TRANSPORT_CONFIG_H
#define MODBUS_GATEWAY_I_TRANSPORT_CONFIG_H

#include <types.h>

namespace modbus_gateway
{

class ITransportConfig
{
public:
     explicit ITransportConfig( TransportType type );

     virtual ~ITransportConfig() = default;

     TransportType GetType() const;

protected:
     TransportType type_;
};

using TransportConfigPtr = std::shared_ptr< ITransportConfig >;

}

#endif
