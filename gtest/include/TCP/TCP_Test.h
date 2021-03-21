#ifndef DIGINEXT_GTEST___TCP_TCP_TEST_H
#define DIGINEXT_GTEST___TCP_TCP_TEST_H

#include <gtest/gtest.h>

#include "TCP/TCP.h"
#include "TCP/TCPClient.h"
#include "TCP/TCPServer.h"

#include <chrono>
#include <list>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/signals2.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace Diginext::Core::TCP::GTest {
    using boost::signals2::signal;
    using namespace boost::asio::ip;
    using namespace std::chrono_literals;

    void init_tcp_test() {
        tcp_all_log_enable();
    }

    tcp::endpoint getLocalEndpointV4(const unsigned port = RANDOM_PORT) {
        auto ip = boost::asio::ip::address::from_string(LOCAL_ADDRESS_TCP_V4);
        return tcp::endpoint(ip, port);
    }

    tcp::endpoint getLocalEndpointV6(const unsigned port = RANDOM_PORT) {
        auto ip = boost::asio::ip::address::from_string(LOCAL_ADDRESS_TCP_V6);
        return tcp::endpoint(ip, port);
    }

    tcp::endpoint getLocalEndpoint(const unsigned port = RANDOM_PORT) {
        return getLocalEndpointV6(port);
    }

    namespace Test_TCP_Server {
        TEST(Test_TCP_Server, Create) {
            tcp::endpoint endpoint = getLocalEndpoint();
            tcp_server *srv;
            ASSERT_NO_THROW({ srv = new tcp_server(endpoint); });
            ASSERT_NE(srv, nullptr);
            ASSERT_NO_THROW({ delete srv; });
        }

        TEST(Test_TCP_Server, Create_Pointer) {
            tcp::endpoint endpoint = getLocalEndpoint();
            tcp_server::pointer srv;
            ASSERT_NO_THROW({ srv = tcp_server::create(endpoint); });
            ASSERT_NE(srv, nullptr);
        }

        TEST(Test_TCP_Server, Start_Stop) {
            using namespace std::chrono_literals;

            tcp::endpoint endpoint = getLocalEndpoint();
            auto srv = tcp_server::create(endpoint);
            ASSERT_NE(srv, nullptr);
            ASSERT_FALSE(srv->started());
            srv->start();
            ASSERT_TRUE(srv->started());
            ASSERT_NE(0, srv->getPort());
            srv->stop();
            ASSERT_FALSE(srv->started());
        }

        TEST(Test_TCP_Server, getConnections_Empty) {
            tcp::endpoint endpoint = getLocalEndpoint();
            auto srv = tcp_server::create(endpoint);

            const auto connections1 = srv->getConnections();
            ASSERT_TRUE(connections1.empty());

            srv->start();
            const auto connections2 = srv->getConnections();
            ASSERT_TRUE(connections2.empty());
            srv->stop();
        }

        TEST(Test_TCP_Server, getConnectionByUUID_Empty) {
            tcp::endpoint endpoint = getLocalEndpoint();
            auto srv = tcp_server::create(endpoint);

            {
                const boost::uuids::uuid boost_uuid = boost::uuids::random_generator()();
                std::string uuid = boost::uuids::to_string(boost_uuid);
                const auto connection = srv->getConnectionByUUID(uuid);
                ASSERT_EQ(nullptr, connection);
            }

            srv->start();

            {
                const boost::uuids::uuid boost_uuid = boost::uuids::random_generator()();
                std::string uuid = boost::uuids::to_string(boost_uuid);
                const auto connection = srv->getConnectionByUUID(uuid);
                ASSERT_EQ(nullptr, connection);
            }

            srv->stop();
        }
    }// namespace Test_TCP_Server

    namespace Test_TCP_Client {
        TEST(Test_TCP_Client, Create) {
            tcp_client *client;
            ASSERT_NO_THROW({ client = new tcp_client(); });
            ASSERT_NE(client, nullptr);
            ASSERT_NO_THROW({ delete client; });
        }

        TEST(Test_TCP_Client, Create_Pointer) {
            tcp_client::pointer client;
            ASSERT_NO_THROW({ client = tcp_client::create(); });
            ASSERT_NE(client, nullptr);
        }
    }// namespace Test_TCP_Client

    namespace Test_TCP_Server_Client {
        TEST(Test_TCP_Server_Client, Connect_1) {
            tcp::endpoint endpoint = getLocalEndpoint();
            auto srv = tcp_server::create(endpoint);

            int server_connections = 0;
            srv->onAccepted.connect([&server_connections](tcp_connection::pointer connection) { server_connections++; });

            tcp::endpoint client_endpoint = getLocalEndpoint(srv->getPort());
            auto client = tcp_client::create();

            bool client1_connected = false;
            client->onConnectionSuccess.connect([&client1_connected](tcp::endpoint &endpoint) { client1_connected = true; });

            srv->start();
            std::this_thread::sleep_for(1s);

            client->connect(client_endpoint);
            client->start();
            std::this_thread::sleep_for(1s);

            auto connections = srv->getConnections();
            ASSERT_FALSE(connections.empty());
            ASSERT_EQ(1, connections.size());
            ASSERT_EQ(1, server_connections);
            ASSERT_TRUE(client1_connected);

            client->stop();
            std::this_thread::sleep_for(1s);

            srv->stop();
            std::this_thread::sleep_for(1s);
        }

        TEST(Test_TCP_Server_Client, Connect_2) {
            int server_connections = 0;

            tcp::endpoint endpoint = getLocalEndpoint();
            auto srv = tcp_server::create(endpoint);

            auto connections = srv->getConnections();
            ASSERT_TRUE(connections.empty());

            srv->onAccepted.connect([&server_connections](tcp_connection::pointer connection) { server_connections++; });

            srv->start();
            std::this_thread::sleep_for(1s);

            connections = srv->getConnections();
            ASSERT_TRUE(connections.empty());

            const unsigned short port = srv->getPort();
            ASSERT_NE(0, port);

            tcp::endpoint client_endpoint = getLocalEndpoint(port);

            bool client1_connected = false;
            bool client2_connected = false;

            auto client1 = tcp_client::create();
            client1->onConnectionSuccess.connect([&client1_connected](tcp::endpoint &endpoint) { client1_connected = true; });
            client1->connect(client_endpoint);
            client1->start();
            std::this_thread::sleep_for(1s);
            ASSERT_EQ(1, server_connections);

            connections = srv->getConnections();
            ASSERT_FALSE(connections.empty());
            ASSERT_EQ(1, connections.size());

            auto client2 = tcp_client::create();
            client2->onConnectionSuccess.connect([&client2_connected](tcp::endpoint &endpoint) { client2_connected = true; });
            client2->connect(client_endpoint);
            client2->start();
            std::this_thread::sleep_for(1s);
            ASSERT_EQ(2, server_connections);

            connections = srv->getConnections();
            ASSERT_FALSE(connections.empty());
            ASSERT_EQ(2, connections.size());

            std::this_thread::sleep_for(3s);

            ASSERT_TRUE(client1_connected);
            ASSERT_TRUE(client2_connected);

            client1->stop();
            client2->stop();
            std::this_thread::sleep_for(5s);

            srv->stop();
        }

        TEST(Test_TCP_Server_Client, Connect_10) {
            tcp::endpoint endpoint = getLocalEndpoint();
            auto srv = tcp_server::create(endpoint);
            srv->start();
            std::this_thread::sleep_for(1s);

            const unsigned short port = srv->getPort();
            tcp::endpoint client_endpoint = getLocalEndpoint(port);

            std::list<tcp_client::pointer> clients;
            for (int i = 0; i < 10; i++) {
                auto client = tcp_client::create();
                clients.push_back(client);
                client->connect(client_endpoint);
                client->start();
            }

            std::this_thread::sleep_for(5s);

            auto connections = srv->getConnections();
            ASSERT_FALSE(connections.empty());
            ASSERT_EQ(10, connections.size());

            for (auto client : clients) {
                client->stop();
            }

            std::this_thread::sleep_for(5s);
            srv->stop();
        }

        TEST(Test_TCP_Server_Client, send___server_1___client_1) {
            std::string client_received_message = std::string();
            std::string server_received_message = std::string();

            tcp::endpoint endpoint = getLocalEndpoint();
            auto srv = tcp_server::create(endpoint);
            srv->onReadMessage.connect([&server_received_message](tcp_connection::pointer connection, std::string msg) {
                server_received_message = msg;
            });

            srv->start();
            std::this_thread::sleep_for(1s);

            const unsigned short port = srv->getPort();
            tcp::endpoint client_endpoint = getLocalEndpoint(port);
            auto client = tcp_client::create();
            client->onReadMessage.connect([&client_received_message](std::string msg) {
                client_received_message = msg;
            });

            client->connect(client_endpoint);
            client->start();
            std::this_thread::sleep_for(1s);

            std::string client_message = "client hello\n abc";
            std::string server_message = "server hello\n\n server";

            auto connections = srv->getConnectionsVector();
            auto connection = connections[0];

            srv->send(connection->getUUID(), server_message);
            client->send(client_message);

            for (int i = 0; i < 5; i++) {
                if (client_received_message.empty() || server_received_message.empty()) {
                    std::this_thread::sleep_for(1s);
                }
            }

            ASSERT_EQ(client_message, server_received_message);
            ASSERT_EQ(server_message, client_received_message);

            client->stop();
            std::this_thread::sleep_for(1s);

            srv->stop();
            std::this_thread::sleep_for(1s);
        }

        inline std::list<string> getTestMessagesList(const int &count = 1, const std::string &prefix = "") {
            std::list<string> messages;
            const std::string TEST_MESSAGE = "test_message";

            for (int i = 0; i < count; i++) {
                const std::string pmsg = prefix.empty() ? "" : (prefix + "_");
                const std::string msg = pmsg + TEST_MESSAGE + "_" + std::to_string(i);
                messages.push_back(msg);
            }

            return messages;
        }

        inline std::vector<string> getTestMessages(const int &count = 1, const std::string &prefix = "") {
            const auto messagesList = getTestMessagesList(count, prefix);
            std::vector<string> messages = std::vector<string>(messagesList.begin(), messagesList.end());
            return messages;
        }

        inline void test___send___client(const int &count = 1) {
            if (count < 1) {
                return;
            }

            const auto messages = getTestMessages(count, "test___send___client");

            std::list<string> messBuffer;

            tcp::endpoint endpoint = getLocalEndpoint();
            auto srv = tcp_server::create(endpoint);
            srv->onReadMessage.connect([&messBuffer](tcp_connection::pointer connection, std::string msg) {
                messBuffer.push_back(msg);
            });

            srv->start();
            std::this_thread::sleep_for(1s);

            const unsigned short port = srv->getPort();
            tcp::endpoint client_endpoint = getLocalEndpoint(port);
            auto client = tcp_client::create();

            client->connect(client_endpoint);
            client->start();
            std::this_thread::sleep_for(1s);

            if (!messages.empty()) {
                for (const auto message : messages) {
                    client->send(message);
                }
            }

            const auto second = std::chrono::seconds(count <= 10 ? 3 : 10);
            std::this_thread::sleep_for(second);

            client->stop();
            std::this_thread::sleep_for(1s);

            srv->stop();
            std::this_thread::sleep_for(1s);
        }

        TEST(Test_TCP_Server_Client, send___client___1) {
            test___send___client(1);
        }

        TEST(Test_TCP_Server_Client, send___client___5) {
            test___send___client(5);
        }

        TEST(Test_TCP_Server_Client, send___client___10) {
            test___send___client(10);
        }

        TEST(Test_TCP_Server_Client, send___client___50) {
            test___send___client(50);
        }

        inline void test___send___server(const int &count = 1) {
            if (count < 1) {
                return;
            }

            const auto messages = getTestMessages(count, "test___send___server");
        }

        TEST(Test_TCP_Server_Client, send___server___1) {
            test___send___server(1);
        }

        TEST(Test_TCP_Server_Client, send___server___5) {
            test___send___server(5);
        }

        TEST(Test_TCP_Server_Client, send___server___10) {
            test___send___server(10);
        }
    }// namespace Test_TCP_Server_Client
}// namespace Diginext::Core::TCP::GTest

#endif
