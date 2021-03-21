#ifndef DIGINEXT_CORE___TCP_TCP_CONNECTION_H
#define DIGINEXT_CORE___TCP_TCP_CONNECTION_H

#include "Log/Log.h"
#include "TCP/TCP.h"

#include <list>
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

    std::vector<std::string> decode_message(
            Logger::pointer logger,
            const std::string &message,
            std::string &message_unreceived_part);

    class tcp_connection : public boost::enable_shared_from_this<tcp_connection> {
    private:
        Logger::pointer logger;

        string uuid;

        boost::asio::io_service *io_service;
        tcp::socket socket_;
        boost::asio::streambuf message_;
        std::string partMessage;

        std::list<string> sendBuffer;
        std::mutex sendSync;
        bool sendStart;

        void handle_connect(const boost::system::error_code &ec, tcp::endpoint &endpoint);
        void handle_read(const boost::system::error_code &error, size_t bytes_transferred);

        void handle_write(const boost::system::error_code &error, size_t bytes_transferred);
        void async_write(bool init = false);

    public:
        typedef boost::shared_ptr<tcp_connection> pointer;

        // create new connection instance for server
        static pointer create(boost::asio::io_service &io_service);

        tcp_connection(boost::asio::io_service &io_service);
        virtual ~tcp_connection();

        void connect(tcp::endpoint &endpoint);

        std::string getUUID();

        tcp::socket &socket();
        void send(std::string msg);

        // start async read
        void start();

        //disconnect
        void stop();
        void disconnect();

        //events
        signal<void(tcp_connection *conn, tcp::endpoint &endpoint)> onConnectionTimedOut;
        signal<void(tcp_connection *conn, tcp::endpoint &endpoint, const boost::system::error_code &ec)> onConnectionError;
        signal<void(tcp_connection *conn, tcp::endpoint &endpoint)> onConnectionSuccess;
        signal<void(tcp_connection *conn)> onDisconnected;
        signal<void(tcp_connection *conn, std::string msg)> onReadMessage;
        signal<void(tcp_connection *conn, const boost::system::error_code error, size_t bytes_transferred)> onReadError;
        signal<void(tcp_connection *conn, const boost::system::error_code error, size_t bytes_transferred)> onSendError;
    };
}// namespace Diginext::Core::TCP

#endif//PP_LIB_AES_TCP_CONNECTION_H
