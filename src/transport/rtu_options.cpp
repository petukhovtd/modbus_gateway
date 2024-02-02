#include <transport/rtu_options.h>

#include <linux/serial.h>

namespace modbus_gateway {

template<typename F, typename R>
void SetOption(F &flags, const std::optional<bool> &option, R flag) {
  if (!option.has_value()) {
    return;
  }
  if (option.value()) {
    flags |= flag;
  } else {
    flags &= ~(flag);
  }
}

void SetOptions(asio::serial_port &serialPort, const RtuOptions &options) {
  serialPort.set_option(options.baudRate);
  serialPort.set_option(options.characterSize);
  serialPort.set_option(options.parity);
  serialPort.set_option(options.stopBits);
  serialPort.set_option(options.flowControl);

  if (options.rs485.has_value()) {
    const auto &rs485Options = options.rs485.value();

    const int fd = serialPort.native_handle();

    serial_rs485 rs485config = {0,};
    if (ioctl(fd, TIOCGRS485, &rs485config) < 0) {
      std::stringstream ss;
      ss << "get rs485 options error: " << std::strerror(errno);
      throw std::runtime_error(ss.str());
    }

    auto &flags = rs485config.flags;
    flags |= SER_RS485_ENABLED;
    SetOption(flags, rs485Options.rtsOnSend, SER_RS485_RTS_ON_SEND);
    SetOption(flags, rs485Options.rtsAfterSend, SER_RS485_RTS_AFTER_SEND);
    SetOption(flags, rs485Options.rxDuringTx, SER_RS485_RX_DURING_TX);
    SetOption(flags, rs485Options.terminateBus, SER_RS485_TERMINATE_BUS);

    if(rs485Options.delayRtsBeforeSend.has_value())
    {
      rs485config.delay_rts_before_send = rs485Options.delayRtsBeforeSend.value();
    }
    if(rs485Options.delayRtsAfterSend.has_value())
    {
      rs485config.delay_rts_after_send = rs485Options.delayRtsAfterSend.value();
    }

    if (ioctl(fd, TIOCSRS485, &rs485config) < 0) {
      std::stringstream ss;
      ss << "set rs485 options error: " << std::strerror(errno);
      throw std::runtime_error(ss.str());
    }
  }
}

}// namespace modbus_gateway
