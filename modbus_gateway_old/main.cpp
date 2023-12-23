#include "command_line.h"
#include "config/config.h"
#include "config/parser_exception.h"
#include <common/fmt_logger.h>
#include "modbus_gateway.h"

#include <iostream>
#include <fstream>

using namespace modbus_gateway;

int main( int argc, char* argv[] )
{
     int rc = EXIT_SUCCESS;
     try
     {
          CommandLine commandLine( argc, argv );

          if( commandLine.IsHelp() )
          {
               commandLine.PrintHelp( std::cout );
               return EXIT_SUCCESS;
          }

          const std::string& configPath = commandLine.GetConfigPath();
          std::ifstream configFile( configPath );
          Config config( configFile );
          config.Validate();

          FmtLogger::SetLogLevel( config.GetLogLevel() );

          rc = ModbusGateway( config );
     }
     catch( const ParserException& e )
     {
          std::cerr << e.what() << '\n'
                    << "in: " << e.GetTargetKey() << '\n'
                    << "path: " << e.GetFullPath() << std::endl;
          rc = EXIT_FAILURE;
     }
     catch( const std::exception& e )
     {
          std::cerr << e.what() << std::endl;
          rc = EXIT_FAILURE;
     }
     catch( ... )
     {
          std::cerr << "unknown exception" << std::endl;
          rc = EXIT_FAILURE;
     }

     return rc;
}