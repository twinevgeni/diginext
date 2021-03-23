#ifndef DIGINEXT_CORE___STORAGE_STORAGE_CLIENT_H
#define DIGINEXT_CORE___STORAGE_STORAGE_CLIENT_H

#include <memory>
#include <string>

#include <Log/Log.h>
#include <TCP/TCP.h>
#include <TCP/TCPClient.h>

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
    * \brief StorageClient
     */
    class StorageClient
    {
    private:
        Logger::pointer logger;
        tcp_client::pointer tcpClient;
        string host;
        unsigned short port;

    public:
        typedef shared_ptr<StorageClient> pointer;
        static pointer create(const string &host = LOCAL_ADDRESS_TCP_V6, const unsigned short port = DEFAULT_PORT);

        StorageClient(const string &host = LOCAL_ADDRESS_TCP_V6, const unsigned short port = DEFAULT_PORT);
        virtual ~StorageClient() = default;

        /**
        * @brief client status
        * \return true | false
        */
        bool Started() const;

        /**
         * @brief connect
         */
        void Connect();

        /**
         * @brief dissconect
         */
        void Disconnect();

        /**
         * @brief send message to server
         * @param[in] msg
         */
        void send(const std::string& msg);

        //handlers
        void handle_connection_timed_oud(tcp::endpoint &endpoint);
        void handle_connection_error(tcp::endpoint &endpoint, const boost::system::error_code &ec);
        void handle_connection_success(tcp::endpoint &endpoint);
        void handle_disconnected();
        void handle_read_message(std::string msg);
        void handle_read_error(const boost::system::error_code error, size_t bytes_transferred);
        void handle_send_error(const boost::system::error_code error, size_t bytes_transferred);
    };
}

#endif
