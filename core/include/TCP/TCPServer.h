#ifndef DIGINEXT_CORE___TCP_TCP_SERVER_H
#define DIGINEXT_CORE___TCP_TCP_SERVER_H

#include "TCP/TCP.h"
#include "TCP/TCPConnection.h"

#include <list>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals2.hpp>

namespace Diginext::Core::TCP {
    using namespace std;
    using namespace Diginext::Core::Log;

    using boost::signals2::signal;
    using namespace boost::asio::ip;

    class tcp_server : public boost::enable_shared_from_this<tcp_server> {
    private:
        std::thread server_thread;
        std::mutex server_sync;

        boost::asio::io_service ios;
        boost::asio::io_service *io_service;
        tcp::acceptor acceptor_;

        bool started_status;
        std::mutex status_sync;

        std::list<tcp_connection::pointer> connections;

        void start_accept();
        void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code &error);

        void removeConnectionByUUID(std::string uuid);
        void removeConnection(tcp_connection::pointer connection);

        bool waitStart();
        bool waitStop();

        //tcp_connection event handlers
        void handle_tcp_connection_disconnected(tcp_connection *connection);
        void handle_tcp_connection_read_message(tcp_connection *connection, std::string msg);
        void handle_tcp_connection_read_error(tcp_connection *connection, const boost::system::error_code error, size_t bytes_transferred);
        void handle_tcp_connection_send_error(tcp_connection *connection, const boost::system::error_code error, size_t bytes_transferred);

    public:
        typedef boost::shared_ptr<tcp_server> pointer;
        static pointer create(tcp::endpoint &endpoint);

        explicit tcp_server(tcp::endpoint &endpoint);
        virtual ~tcp_server();

        tcp::acceptor *getAcceptor();

        void start();
        void stop();
        bool started();

        tcp::endpoint getLocalEndpoint() const;
        std::string getLocalAddress() const;
        unsigned short getPort() const;

        std::list<tcp_connection::pointer> getConnections();
        std::vector<tcp_connection::pointer> getConnectionsVector();
        tcp_connection::pointer getConnectionByUUID(std::string uuid);

        void disconnectByUUID(std::string uuid);
        void disconnect(tcp_connection::pointer connection);
        void disconnectAll();

        void send(tcp_connection::pointer connection, std::string message);
        void send(std::string uuid, std::string message);
        void sendAll(std::string message);

        //events
        signal<void(tcp_connection::pointer connection)> onAccepted;
        signal<void(tcp_connection::pointer connection, const boost::system::error_code error)> onAcceptError;
        signal<void(tcp_connection::pointer connection)> onDisconnected;
        signal<void(tcp_connection::pointer connection, std::string msg)> onReadMessage;
        signal<void(tcp_connection::pointer connection, const boost::system::error_code error, size_t bytes_transferred)> onReadError;
        signal<void(tcp_connection::pointer connection, const boost::system::error_code error, size_t bytes_transferred)> onSendError;
    };
}// namespace Diginext::Core::TCP

#endif