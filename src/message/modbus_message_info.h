#pragma once

#include <modbus/modbus_types.h>

#include <exchange/id.h>

#include <chrono>

namespace modbus_gateway {

class ModbusMessageInfo {
    using TargetClock = std::chrono::steady_clock;

public:
    ModbusMessageInfo(exchange::ActorId sourceId, modbus::TransactionId transactionId);

    virtual ~ModbusMessageInfo() = default;

    bool TimeoutReached(std::chrono::nanoseconds timeout) const;

    exchange::ActorId GetSourceId() const;

    modbus::TransactionId GetTransactionId() const;

private:
    static std::chrono::nanoseconds GetCurrentTimestamp();

private:
    std::chrono::nanoseconds createTimeStamp_;
    mutable bool timeoutReached_;
    exchange::ActorId sourceId_;
    modbus::TransactionId transactionId_;
};

}
