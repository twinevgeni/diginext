#include "Storage/StorageClient.h"

#include "Log/LogConsole.h"

#include <chrono>

namespace Diginext::Core::Storage {
    using namespace std::chrono_literals;

    StorageClient::pointer StorageClient::create(const string &host, const unsigned short port) {
        return std::make_shared<StorageClient>(host, port);
    }

    StorageClient::StorageClient(const string &host, const unsigned short port) {
        this->logger = ConsoleLogger::create("StorageClient", false);
        this->host = host;
        this->port = port;
        this->tcpClient = tcp_client::create();
        this->tcpClient->onConnectionTimedOut.connect(boost::bind(&StorageClient::handle_connection_timed_oud, this, _1));
        this->tcpClient->onConnectionError.connect(boost::bind(&StorageClient::handle_connection_error, this, _1, _2));
        this->tcpClient->onConnectionSuccess.connect(boost::bind(&StorageClient::handle_connection_success, this, _1));
        this->tcpClient->onDisconnected.connect(boost::bind(&StorageClient::handle_disconnected, this));
        this->tcpClient->onReadMessage.connect(boost::bind(&StorageClient::handle_read_message, this, _1));
        this->tcpClient->onReadError.connect(boost::bind(&StorageClient::handle_read_error, this, _1, _2));
        this->tcpClient->onSendError.connect(boost::bind(&StorageClient::handle_send_error, this, _1, _2));
    }

    bool StorageClient::Started() const {
        try {
            if (this->tcpClient != nullptr) {
                return this->tcpClient->started();
            }
        } catch (...) {
        }

        return false;
    }

    void StorageClient::Connect() {
        if (this->Started())
        {
            return;
        }

        const auto ip = boost::asio::ip::address::from_string(this->host);
        auto endpoint = tcp::endpoint(ip, this->port);
        this->tcpClient->connect(endpoint);
        this->tcpClient->start();
    }

    void StorageClient::Disconnect() {
        if (!this->Started())
        {
            return;
        }

        this->tcpClient->disconnect();
        this->tcpClient->stop();
    }

    void StorageClient::send(const std::string& msg)
    {
        this->tcpClient->send(msg);
    }

    string StorageClient::getAnswer() const
    {
        return this->answer;
    }

    void StorageClient::handle_connection_timed_oud(tcp::endpoint &endpoint) {
        this->logger->LogInfo("client | connection time out");
    }

    void StorageClient::handle_connection_error(tcp::endpoint &endpoint, const boost::system::error_code &ec) {
        this->logger->LogInfo("client | connection error: " + ec.message());
    }

    void StorageClient::handle_connection_success(tcp::endpoint &endpoint) {
        this->logger->LogInfo("client | connection success");
    }

    void StorageClient::handle_disconnected() {
        this->logger->LogInfo("client | disconnected");
    }

    void StorageClient::handle_read_message(std::string msg) {
        this->logger->LogInfo("client | new message read | msg: " + msg);
        this->answer = msg;
    }

    void StorageClient::handle_read_error(const boost::system::error_code error, size_t bytes_transferred) {
        this->logger->LogInfo("client | read error: " + error.message());
    }

    void StorageClient::handle_send_error(const boost::system::error_code error, size_t bytes_transferred) {
        this->logger->LogInfo("client | send error: " + error.message());
    }
}// namespace Diginext::Core::Storage