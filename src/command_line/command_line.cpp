#include <command_line/command_line.h>
#include <command_line/command_line_exception.h>

namespace modbus_gateway {

CommandLine::CommandLine(int argc, char **argv)
    : options_("modbus_gateway", "Modbus gateway") {
  options_.add_options()("h,help", "Print usage")("c,config", "Config file", cxxopts::value<std::string>());

  parseResult_ = options_.parse(argc, argv);
}

bool CommandLine::IsHelp() const {
  return parseResult_.count("help");
}

void CommandLine::PrintHelp(std::ostream &os) const {
  os << options_.help();
}

const std::string &CommandLine::GetConfigPath() const {
  if (parseResult_.count("config")) {
    return parseResult_["config"].as<std::string>();
  }

  throw CommandLineException("config arg not set");
}

}// namespace modbus_gateway