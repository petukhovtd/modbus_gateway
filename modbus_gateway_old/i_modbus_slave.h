#ifndef MODBUS_GATEWAY_I_MODBUS_SLAVE_H
#define MODBUS_GATEWAY_I_MODBUS_SLAVE_H

#include <types.h>

namespace modbus_gateway
{

class IModbusSlave
{
public:
     explicit IModbusSlave( TransportType type );

     virtual ~IModbusSlave() = default;

     TransportType GetType() const;

     virtual void Start() = 0;

private:
     TransportType type_;
};

using ModbusSlavePtr = std::shared_ptr< IModbusSlave >;

}

#endif //MODBUS_GATEWAY_I_MODBUS_SLAVE_H
