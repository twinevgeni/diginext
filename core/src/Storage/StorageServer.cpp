#include "Storage/StorageServer.h"

#include "Log/LogConsole.h"

#include <chrono>

#include <nlohmann/json.hpp>

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
        if (this->dataStorage.find(key) != this->dataStorage.end()) {
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
        this->logger->LogInfo("server | accept new connection with uuid: " + connection->getUUID());
    }

    void StorageServer::handle_accept_error(tcp_connection::pointer connection, const boost::system::error_code error) {
        this->logger->LogInfo("server | accept error with uuid: " + connection->getUUID());
    }

    void StorageServer::handle_disconnect(tcp_connection::pointer connection) {
        this->logger->LogInfo("server | client disconnected | uuid: " + connection->getUUID());
    }

    void StorageServer::sendErrorStatus(tcp_connection::pointer connection, const string& description)
    {
        nlohmann::json json;
        json[JSON::KEY::STATUS] = JSON::VALUE::STATUS_ERROR;
        json[JSON::KEY::DESCRIPTION] = description;

        std::string jsonString = json.dump();
        connection->send(jsonString);
    }

    void StorageServer::sendOkRead(tcp_connection::pointer connection, const string& value)
    {
        nlohmann::json json;
        json[JSON::KEY::STATUS] = JSON::VALUE::STATUS_OK;
        json[JSON::KEY::VALUE] = value;

        std::string jsonString = json.dump();
        connection->send(jsonString);
    }

    void StorageServer::sendOkWrite(tcp_connection::pointer connection)
    {
        nlohmann::json json;
        json[JSON::KEY::STATUS] = JSON::VALUE::STATUS_OK;

        std::string jsonString = json.dump();
        connection->send(jsonString);
    }

    void StorageServer::handle_read_message(tcp_connection::pointer connection, std::string msg) {
        this->logger->LogInfo("server | new message from client | uuid: " + connection->getUUID() + " | msg: " + msg);

        try {
            nlohmann::json json = nlohmann::json::parse(msg);
            if (!json.contains(JSON::KEY::REQUEST)) {
                this->sendErrorStatus(connection, "field" + JSON::KEY::REQUEST + " not found");
            }

            if (!json.contains(JSON::KEY::KEY)) {
                this->sendErrorStatus(connection, "field" + JSON::KEY::KEY + " not found");
            }

            const std::string request = json[JSON::KEY::REQUEST].get<string>();
            const std::string key = json[JSON::KEY::KEY].get<string>();

            if (request == JSON::VALUE::REQUEST_READ)
            {
                if (this->keyExists(key)) {
                    const std::string value = this->readValue(key);
                    this->sendOkRead(connection, value);
                } else {
                    this->sendErrorStatus(connection, "key not found in storage");
                }

            } else if (request == JSON::VALUE::REQUEST_WRITE)
            {
                if (!json.contains(JSON::KEY::VALUE)) {
                    this->sendErrorStatus(connection, "field" + JSON::KEY::KEY + " not found");
                }

                const  std::string value = json[JSON::KEY::VALUE].get<string>();
                this->writeValue(key, value);
                this->sendOkWrite(connection);

            } else {
                this->sendErrorStatus(connection, "request must be read/write");
            }

        } catch (...) {
            this->sendErrorStatus(connection, "json parse error");
        }

        connection->disconnect();
    }

    void StorageServer::handle_read_error(tcp_connection::pointer connection, const boost::system::error_code error, size_t bytes_transferred) {
        this->logger->LogInfo("server | msg read error | uuid: " + connection->getUUID() + " | error: " + error.message());
    }

    void StorageServer::handle_send_error(tcp_connection::pointer connection, const boost::system::error_code error, size_t bytes_transferred) {
        this->logger->LogInfo("server | msg send error | uuid: " + connection->getUUID() + " | error: " + error.message());
    }
}// namespace Diginext::Core::Storage