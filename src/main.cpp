#include "command_line/include/command_line/command_line.h"
#include <common/logger.h>
#include <config/config.h>
#include <config/parser_exception.h>

#include <modbus_gateway.h>

#include <fstream>
#include <iostream>

using namespace modbus_gateway;

int main(int argc, char *argv[]) {

  int rc = EXIT_SUCCESS;
  try {

    CommandLine commandLine(argc, argv);

    if (commandLine.IsHelp()) {
      commandLine.PrintHelp(std::cout);
      return EXIT_SUCCESS;
    }

    const std::string &configPath = commandLine.GetConfigPath();
    std::ifstream configFile(configPath);
    Config config(configFile);
    config.Validate();

    rc = ModbusGateway(config);

  } catch (const ParserException &e) {

    std::cerr << "error: " << e.what() << '\n'
              << "\tin: " << e.GetTargetKey() << '\n'
              << "\tpath: " << e.GetFullPath() << std::endl;
    rc = EXIT_FAILURE;

  } catch (const std::exception &e) {

    std::cerr << "error: " << e.what() << std::endl;
    rc = EXIT_FAILURE;

  } catch (...) {

    std::cerr << "error: unknown exception" << std::endl;
    rc = EXIT_FAILURE;

  }

  return rc;
}