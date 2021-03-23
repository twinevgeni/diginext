#ifndef DIGINEXT_CORE___STORAGE_STORAGE_SERVER_H
#define DIGINEXT_CORE___STORAGE_STORAGE_SERVER_H

#include <memory>
#include <string>

#include <Log/Log.h>
#include <TCP/TCP.h>
#include <TCP/TCPServer.h>

#include <map>
#include <mutex>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace Diginext::Core::Storage {

    using namespace std;
    using namespace Diginext::Core::TCP;
    using namespace Diginext::Core::Log;

    /**
     * \brief StorageServer
     */
    class StorageServer {
    private:
        Logger::pointer logger;
        tcp_server::pointer tcpServer;
        std::map<string, string> dataStorage;
        std::mutex dataSync;

        bool keyExists(string key);
        string readValue(string key);
        void writeValue(string key, string value);

    public:
        typedef shared_ptr<StorageServer> pointer;
        static pointer create(const string &host = LOCAL_ADDRESS_TCP_V6, const unsigned short port = DEFAULT_PORT);

        StorageServer(const string &host = LOCAL_ADDRESS_TCP_V6, const unsigned short port = DEFAULT_PORT);
        virtual ~StorageServer();

        /**
         * @brief server status
         * @return server status
         */
        bool Started() const;

        /**
         * @brief server listen addr
         * @return server listen addr
         */
        std::string getAddress() const;

        /**
         * @brief server listen port
         * @return server listen port
         */
        unsigned short getPort() const;

        /**
         * \brief start server
         */
        void Start();

        /**
         * \brief stop server
         */
        void Stop();

        //handlers
        void handle_accept(tcp_connection::pointer connection);
        void handle_accept_error(tcp_connection::pointer connection, const boost::system::error_code error);
        void handle_disconnect(tcp_connection::pointer connection);
        void handle_read_message(tcp_connection::pointer connection, std::string msg);
        void handle_read_error(tcp_connection::pointer connection, const boost::system::error_code error, size_t bytes_transferred);
        void handle_send_error(tcp_connection::pointer connection, const boost::system::error_code error, size_t bytes_transferred);
    };
}// namespace Diginext::Core::Storage

#endif
