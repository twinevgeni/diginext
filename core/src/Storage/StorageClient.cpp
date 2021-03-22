#include "Storage/StorageClient.h"

#include "Log/LogConsole.h"

#include <chrono>

namespace Diginext::Core::Storage {
    using namespace std::chrono_literals;

    StorageClient::pointer StorageClient::create(const string &host, const unsigned short port) {
        return std::make_shared<StorageClient>(host, port);
    }

    StorageClient::StorageClient(const string &host, const unsigned short port) {
        const auto ip = boost::asio::ip::address::from_string(host);
        auto endpoint = tcp::endpoint(ip, port);
        this->tcpClient = tcp_client::create();
    }

    bool StorageClient::Started() const {
        try
        {
            if (this->tcpClient != nullptr) {
                return this->tcpClient->started();
            }
        }
        catch (...)
        {
        }

        return false;
    }
}// namespace Diginext::Core::Storage