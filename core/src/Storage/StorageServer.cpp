#include "Storage/StorageServer.h"

#include "Log/LogConsole.h"

#include <chrono>

namespace Diginext::Core::Storage {
    using namespace std::chrono_literals;

    StorageServer::pointer StorageServer::create(const string &host, const unsigned short port) {
        return std::make_shared<StorageServer>(host, port);
    }

    StorageServer::StorageServer(const string &host, const unsigned short port) {
        this->logger = ConsoleLogger::create("StorageServer");

        auto ip = boost::asio::ip::address::from_string(host);
        auto endpoint = tcp::endpoint(ip, port);
        this->tcpServer = tcp_server::create(endpoint);

        this->tcpServer->onAccepted.connect(boost::bind(&StorageServer::handle_accept, this, _1));
        this->tcpServer->onAcceptError.connect(boost::bind(&StorageServer::handle_accept_error, this, _1, _2));
        this->tcpServer->onDisconnected.connect(boost::bind(&StorageServer::handle_disconnect, this, _1));
        this->tcpServer->onReadMessage.connect(boost::bind(&StorageServer::handle_read_message, this, _1, _2));
        this->tcpServer->onReadError.connect(boost::bind(&StorageServer::handle_read_error, this, _1, _2, _3));
        this->tcpServer->onSendError.connect(boost::bind(&StorageServer::handle_send_error, this, _1, _2, _3));
    }

    StorageServer::~StorageServer() {
    }

    bool StorageServer::keyExists(string key) {
        std::lock_guard<std::mutex> guard(this->dataSync);
        return this->dataStorage.find(key) != this->dataStorage.end();
    }

    string StorageServer::readValue(string key) {
        std::lock_guard<std::mutex> guard(this->dataSync);
        if (this->keyExists(key)) {
            return this->dataStorage[key];
        }

        return std::string();
    }

    void StorageServer::writeValue(string key, string value) {
        std::lock_guard<std::mutex> guard(this->dataSync);
        this->dataStorage[key] = value;
    }

    bool StorageServer::Started() const {
        try {
            if (this->tcpServer != nullptr) {
                return this->tcpServer->started();
            }
        } catch (...) {
        }

        return false;
    }

    std::string StorageServer::getAddress() const {
        return this->tcpServer->getLocalAddress();
    }

    unsigned short StorageServer::getPort() const {
        return this->tcpServer->getPort();
    }

    void StorageServer::Start() {
        this->logger->LogInfo("... starting server ...");
        if (this->tcpServer != nullptr && this->tcpServer->started()) {
            logger->LogInfo("server already started");
            return;
        }

        this->tcpServer->start();
        std::this_thread::sleep_for(5s);

        this->logger->LogInfo("... addr: " + this->getAddress());
        this->logger->LogInfo("... port: " + std::to_string(this->getPort()));

        this->logger->LogInfo("... started server ...");
    }

    void StorageServer::Stop() {
        this->logger->LogInfo("... stopping ...");
    }

    void StorageServer::handle_accept(tcp_connection::pointer connection) {
        this->logger->LogInfo("tcp | accept new connection with uuid: " + connection->getUUID());
    }

    void StorageServer::handle_accept_error(tcp_connection::pointer connection, const boost::system::error_code error) {
        this->logger->LogInfo("tcp | accept error with uuid: " + connection->getUUID());
    }

    void StorageServer::handle_disconnect(tcp_connection::pointer connection) {
        this->logger->LogInfo("tcp | client disconnected | uuid: " + connection->getUUID());
    }

    void StorageServer::handle_read_message(tcp_connection::pointer connection, std::string msg) {
        this->logger->LogInfo("tcp | new message from client | uuid: " + connection->getUUID() + " | msg: " + msg);
    }

    void StorageServer::handle_read_error(tcp_connection::pointer connection, const boost::system::error_code error, size_t bytes_transferred) {
        this->logger->LogInfo("tcp | msg read error | uuid: " + connection->getUUID() + " | error: " + error.message());
    }

    void StorageServer::handle_send_error(tcp_connection::pointer connection, const boost::system::error_code error, size_t bytes_transferred) {
        this->logger->LogInfo("tcp | msg send error | uuid: " + connection->getUUID() + " | error: " + error.message());
    }
}// namespace Diginext::Core::Storage