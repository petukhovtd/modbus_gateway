#include <common/logger.h>

#include <common/types_asio.h>

#include <vector>

int main(int argc, char *argv[]) {
  asio::io_context context = asio::io_context();
  std::string device = "/dev/pts/4";
  asio::serial_port serialPort(context, device);
  serialPort.set_option(asio::serial_port_base::baud_rate(9600));

  std::vector<int> buf = {41,42};
  serialPort.write_some(asio::buffer(buf));
  return 0;
}