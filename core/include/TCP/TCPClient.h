#ifndef DIGINEXT_CORE___TCP_TCP_CLIENT_H
#define DIGINEXT_CORE___TCP_TCP_CLIENT_H

#include "TCP/TCP.h"
#include "TCP/TCPConnection.h"

#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

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

    class tcp_client : public boost::enable_shared_from_this<tcp_client> {
    private:
        std::thread server_thread;
        std::mutex server_sync;

        boost::asio::io_service ios;
        boost::asio::io_service *io_service;
        tcp_connection::pointer tcp_conn;

        bool started_status;
        std::mutex status_sync;

        bool waitStart();
        bool waitStop();

        //handlers
        void handle_tcp_connection_timeout(tcp_connection *connection, tcp::endpoint &endpoint);
        void handle_tcp_connection_error(tcp_connection *connection, tcp::endpoint &endpoint, const boost::system::error_code &ec);
        void handle_tcp_connection_success(tcp_connection *connection, tcp::endpoint &endpoint);
        void handle_tcp_connection_disconnect(tcp_connection *connection);
        void handle_tcp_connection_read_message(tcp_connection *connection, std::string msg);
        void handle_tcp_connection_read_error(tcp_connection *connection, const boost::system::error_code error, size_t bytes_transferred);
        void handle_tcp_connection_send_error(tcp_connection *connection, const boost::system::error_code error, size_t bytes_transferred);

    public:
        typedef boost::shared_ptr<tcp_client> pointer;
        static pointer create();

        tcp_connection::pointer getConnection();
        void connect(tcp::endpoint &endpoint);
        void disconnect();
        void send(std::string msg);

        tcp_client();
        virtual ~tcp_client();

        void start();
        void stop();
        bool started();

        //events
        signal<void(tcp::endpoint &endpoint)> onConnectionTimedOut;
        signal<void(tcp::endpoint &endpoint, const boost::system::error_code &ec)> onConnectionError;
        signal<void(tcp::endpoint &endpoint)> onConnectionSuccess;
        signal<void()> onDisconnected;
        signal<void(std::string msg)> onReadMessage;
        signal<void(const boost::system::error_code error, size_t bytes_transferred)> onReadError;
        signal<void(const boost::system::error_code error, size_t bytes_transferred)> onSendError;
    };
}// namespace Diginext::Core::TCP

#endif