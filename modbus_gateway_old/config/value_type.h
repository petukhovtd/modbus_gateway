#ifndef MODBUS_GATEWAY_VALUE_TYPE_H
#define MODBUS_GATEWAY_VALUE_TYPE_H

#include <nlohmann/json.hpp>

namespace modbus_gateway
{

enum ValueType
{
     Unknown,
     Object,
     Array,
     String,
     Boolean,
     Number,
};

ValueType ConvertJsonTypeToValueType( nlohmann::json::value_t type );

std::string ConvertValueTypeToString( ValueType valueType );

}

#endif
