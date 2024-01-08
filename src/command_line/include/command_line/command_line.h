#pragma once

#include <string>

#include <cxxopts.hpp>

namespace modbus_gateway {

class CommandLine {
public:
  CommandLine(int argc, char *argv[]);

  bool IsHelp() const;

  void PrintHelp(std::ostream &os) const;

  const std::string &GetConfigPath() const;

private:
  cxxopts::Options options_;
  cxxopts::ParseResult parseResult_;
};

}// namespace modbus_gateway
