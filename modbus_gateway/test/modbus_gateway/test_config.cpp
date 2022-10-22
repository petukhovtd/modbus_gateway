#include <gtest/gtest.h>
#include <config/config.h>
#include <config/i_transport_config.h>
#include <config/validators.h>
#include <config/invalid_value_exception.h>
#include <config/tcp_server_config.h>
#include <config/tcp_client_config.h>

#include <sstream>

TEST( ConfigTest, ConfigReadTest )
{
     std::stringstream is;
     is << R"(
{
  "log_level": "trace",
  "servers": [
    {
      "transport_type": "tcp",
      "ip_port": 502
    },
    {
      "transport_type": "tcp",
      "ip_address": "10.10.10.22",
      "ip_port": 1234
    }
  ],
  "clients": [
    {
      "transport_type": "tcp",
      "ip_address": "10.10.20.22",
      "ip_port": 502,
      "unit_id": [
        {
          "type": "range",
          "begin": 1,
          "end": 127
        },
        {
          "type": "value",
          "value": 130
        }
      ]
    },
    {
      "transport_type": "tcp",
      "ip_address": "10.10.20.33",
      "ip_port": 502
    }
  ]
}
)";
     std::optional< modbus_gateway::Config > config;
     ASSERT_NO_THROW( config = modbus_gateway::Config( is ) );
     ASSERT_TRUE( config );
     EXPECT_NO_THROW( config->Validate() );
}

TEST( ConfigTest, ValidateServersTest )
{
     namespace mg = modbus_gateway;
     {
          std::vector< mg::TransportConfigPtr > servers = {
          std::make_shared< mg::TcpServerConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 502 ),
          std::make_shared< mg::TcpServerConfig >( asio::ip::make_address_v4( "127.0.0.2" ), 502 ),
          std::make_shared< mg::TcpServerConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 503 ),
          std::make_shared< mg::TcpServerConfig >( asio::ip::make_address_v4( "127.0.0.2" ), 503 ),
          };
          mg::TracePath tracePath;
          EXPECT_NO_THROW( mg::ValidateServers( tracePath, servers ) );
     }
     {
          std::vector< mg::TransportConfigPtr > servers = {
          std::make_shared< mg::TcpServerConfig >( 502 ),
          std::make_shared< mg::TcpServerConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 503 ),
          std::make_shared< mg::TcpServerConfig >( asio::ip::make_address_v4( "127.0.0.2" ), 503 ),
          };
          mg::TracePath tracePath;
          EXPECT_NO_THROW( mg::ValidateServers( tracePath, servers ) );
     }
     {
          std::vector< mg::TransportConfigPtr > servers = {
          std::make_shared< mg::TcpServerConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 502 ),
          std::make_shared< mg::TcpServerConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 502 ),
          };
          mg::TracePath tracePath;
          EXPECT_THROW( mg::ValidateServers( tracePath, servers ), mg::InvalidValueException );
     }
     {
          std::vector< mg::TransportConfigPtr > servers = {
          std::make_shared< mg::TcpServerConfig >( 502 ),
          std::make_shared< mg::TcpServerConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 502 ),
          };
          mg::TracePath tracePath;
          EXPECT_THROW( mg::ValidateServers( tracePath, servers ), mg::InvalidValueException );
     }
     {
          std::vector< mg::TransportConfigPtr > servers = {
          std::make_shared< mg::TcpServerConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 502 ),
          std::make_shared< mg::TcpServerConfig >( 502 ),
          };
          mg::TracePath tracePath;
          EXPECT_THROW( mg::ValidateServers( tracePath, servers ), mg::InvalidValueException );
     }
}

TEST( ConfigTest, ValidateClientTest )
{
     namespace mg = modbus_gateway;
     using UnitIdRanges = std::vector< mg::UnitIdRange >;
     {
          std::vector< mg::TransportConfigPtr > clients = {
          std::make_shared< mg::TcpClientConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 502,
                                                   UnitIdRanges { mg::UnitIdRange( 1 ) } ),
          std::make_shared< mg::TcpClientConfig >( asio::ip::make_address_v4( "127.0.0.2" ), 502,
                                                   UnitIdRanges { mg::UnitIdRange( 2 ) } ),
          };
          mg::TracePath tracePath;
          EXPECT_NO_THROW( mg::ValidateClients( tracePath, clients ) );
     }
     {
          std::vector< mg::TransportConfigPtr > clients = {
          std::make_shared< mg::TcpClientConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 502,
                                                   UnitIdRanges { mg::UnitIdRange( 1 ) } ),
          std::make_shared< mg::TcpClientConfig >( asio::ip::make_address_v4( "127.0.0.2" ), 502,
                                                   UnitIdRanges{} ),
          };
          mg::TracePath tracePath;
          EXPECT_NO_THROW( mg::ValidateClients( tracePath, clients ) );
     }
     {
          std::vector< mg::TransportConfigPtr > clients = {
          std::make_shared< mg::TcpClientConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 502,
                                                   UnitIdRanges { mg::UnitIdRange( 1 ) } ),
          std::make_shared< mg::TcpClientConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 502,
                                                   UnitIdRanges { mg::UnitIdRange( 2 ) } ),
          };
          mg::TracePath tracePath;
          EXPECT_THROW( mg::ValidateClients( tracePath, clients ), mg::InvalidValueException );
     }
     {
          std::vector< mg::TransportConfigPtr > clients = {
          std::make_shared< mg::TcpClientConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 502,
                                                   UnitIdRanges { mg::UnitIdRange( 1 ) } ),
          std::make_shared< mg::TcpClientConfig >( asio::ip::make_address_v4( "127.0.0.2" ), 502,
                                                   UnitIdRanges { mg::UnitIdRange( 1, 5 ) } ),
          };
          mg::TracePath tracePath;
          EXPECT_THROW( mg::ValidateClients( tracePath, clients ), mg::InvalidValueException );
     }
     {
          std::vector< mg::TransportConfigPtr > clients = {
          std::make_shared< mg::TcpClientConfig >( asio::ip::make_address_v4( "127.0.0.1" ), 502,
                                                   UnitIdRanges {} ),
          std::make_shared< mg::TcpClientConfig >( asio::ip::make_address_v4( "127.0.0.2" ), 502,
                                                   UnitIdRanges {} ),
          };
          mg::TracePath tracePath;
          EXPECT_THROW( mg::ValidateClients( tracePath, clients ), mg::InvalidValueException );
     }
}