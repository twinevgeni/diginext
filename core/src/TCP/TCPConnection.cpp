#include "TCP/TCPConnection.h"

#include "Base64/Base64.h"
#include "Log/LogConsole.h"

#include <mutex>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/algorithm/count.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace Diginext::Core::TCP {
    const char DELIMETR = '\n';
    const std::string DELIMETR_STR = std::string(1, DELIMETR);

    tcp_connection::pointer tcp_connection::create(boost::asio::io_service &io_service) {
        return boost::make_shared<tcp_connection>(io_service);
    }

    tcp_connection::tcp_connection(boost::asio::io_service &io_service)
        : socket_(io_service) {
        this->io_service = &io_service;

        const boost::uuids::uuid boost_uuid = boost::uuids::random_generator()();
        this->uuid = boost::uuids::to_string(boost_uuid);

        this->logger = ConsoleLogger::create("tcp_connection_" + this->uuid);
        this->logger->SetEnabled(tcp_connection_log_enabled());
        this->logger->LogInfo("tcp_connection | object created");
    }

    tcp_connection::~tcp_connection() {
    }

    void tcp_connection::connect(tcp::endpoint &endpoint) {
        this->logger->LogInfo("tcp_connection::connect | host: " + endpoint.address().to_string() + " | port: " + std::to_string(endpoint.port()));
        this->socket_.async_connect(endpoint, boost::bind(&tcp_connection::handle_connect, this, _1, endpoint));
    }

    void tcp_connection::handle_connect(const boost::system::error_code &ec, tcp::endpoint &endpoint) {
        this->logger->LogInfo("tcp_connection::handle_connect | method call");

        if (!socket_.is_open()) {
            this->logger->LogInfo("tcp_connection::handle_connect | Connect timed out");
            this->onConnectionTimedOut(this, endpoint);
        } else if (ec) {
            this->logger->LogInfo("tcp_connection::handle_connect | Connect error: " + ec.message());
            socket_.close();
            this->onConnectionError(this, endpoint, ec);
        } else {
            this->logger->LogInfo("tcp_connection::handle_connect | connected successfully");
            start();
            this->onConnectionSuccess(this, endpoint);
        }
    }

    std::vector<std::string> decode_message(
            Logger::pointer logger,
            const std::string& message,
            std::string& message_unreceived_part)
    {
        logger->LogInfo("tcp_connection::decode_message | method called");

        std::vector<std::string> messages;

        if (!message.empty())
        {
            logger->LogInfo("tcp_connection::decode_message | decoding message");

            const auto delimeterCount = boost::count(message, DELIMETR);
            logger->LogInfo("tcp_connection::decode_message | decoding message | delimeter_count: " + std::to_string(delimeterCount));

            std::string messageFull;
            messageFull = message;
            if (messageFull.back() != DELIMETR)
            {
                const auto lastPos = messageFull.find_last_of(DELIMETR);
                message_unreceived_part = messageFull.substr(lastPos + 1);
                messageFull = messageFull.substr(0, lastPos + 1);

                logger->LogInfo("tcp_connection::decode_message | decoding message | detected message_unreceived_part: " + message_unreceived_part);
                logger->LogInfo("tcp_connection::decode_message | decoding message | message_received: " + messageFull);
            }

            std::vector<std::string> message_vector;
            boost::split(message_vector, messageFull, boost::is_any_of(DELIMETR_STR));
            const auto vector_size = message_vector.size();
            logger->LogInfo("tcp_connection::decode_message | message_vector prepared | size " + std::to_string(vector_size));

            int index = 0;
            for (const std::string& msg : message_vector)
            {
                const std::string element_pos = " | index: " + std::to_string(index) + " of " + std::to_string(vector_size);

                logger->LogDebug("tcp_connection::decode_message | message_vector element : " + msg + element_pos);
                if (!msg.empty())
                {
                    try
                    {
                        const std::string msgDecoded = Base64::Decode(msg);
                        logger->LogDebug("tcp_connection::decode_message | message_vector element decoded : " + msgDecoded + element_pos);
                        messages.push_back(msgDecoded);
                    }
                    catch (...)
                    {
                        logger->LogError("tcp_connection::decode_message | decode error : " + msg + element_pos);
                    }
                }

                index++;
            }
        }

        return messages;
    }

    void tcp_connection::handle_read(const boost::system::error_code &error, size_t bytes_transferred) {
        this->logger->LogInfo("tcp_connection::handle_read | handler called");

        if (error) {
            if ((boost::asio::error::eof == error) ||
                (boost::asio::error::connection_reset == error)) {
                this->logger->LogInfo("tcp_connection::handle_read | client disconnected");
                this->onDisconnected(this);
                return;
            }

            this->logger->LogError(error.message());
            this->onReadError(this, error, bytes_transferred);
            return;
        }

        this->logger->LogInfo("tcp_connection::handle_read | reading message from buffer");
        std::string msg;

        {
            std::stringstream ss;
            ss << &message_;
            ss.flush();
            msg = ss.str();
        }

        msg = this->partMessage + msg;
        this->partMessage = "";
        this->logger->LogInfo("tcp_connection::handle_read | message: " + msg);

        if (!msg.empty()) {
            std::string unreceived_part = "";
            const auto messages = decode_message(this->logger, msg, unreceived_part);

            for (const std::string &msg : messages) {
                this->logger->LogDebug("tcp_connection::handle_read | sending event onReadMessage : " + msg);
                this->onReadMessage(this, msg);
            }

            this->partMessage = unreceived_part;
        } else {
            this->logger->LogInfo("tcp_connection::handle_read | empty message");
        }

        this->logger->LogInfo("tcp_connection::handle_read | calling async_read_until");
        boost::asio::async_read_until(
                socket_,
                message_,
                '\n',
                boost::bind(&tcp_connection::handle_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    std::string tcp_connection::getUUID() {
        return this->uuid;
    }

    tcp::socket &tcp_connection::socket() {
        return socket_;
    }

    void tcp_connection::handle_write(const boost::system::error_code &error, size_t bytes_transferred) {
        this->logger->LogInfo("tcp_connection::handle_write | handler called");

        if (error) {
            try {
                this->logger->LogError("tcp_connection::handle_write | onSendError : " + error.message());
                this->onSendError(this, error, bytes_transferred);
            } catch (...) {
            }
        }

        this->async_write();
    }

    void tcp_connection::async_write(bool init) {
        std::lock_guard<std::mutex> guard(this->sendSync);

        this->logger->LogInfo("tcp_connection::async_write | method called");
        std::list<string>::iterator msg_itertor;

        if (init) {
            if (this->sendStart) {
                return;
            } else {
                this->sendStart = true;
            }
        } else {
            try {
                if (!this->sendBuffer.empty()) {
                    this->sendBuffer.pop_front();
                }
            } catch (...) {
            }
        }

        if (this->sendBuffer.empty()) {
            this->sendStart = false;
            return;
        }

        msg_itertor = this->sendBuffer.begin();

        boost::asio::async_write(
                this->socket(),
                boost::asio::buffer(msg_itertor->c_str(), msg_itertor->size()),
                boost::bind(
                        &tcp_connection::handle_write,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }

    void tcp_connection::send(std::string msg)
    {
        this->logger->LogInfo("tcp_connection::send | method called");

        {
            std::lock_guard<std::mutex> guard(this->sendSync);
            try
            {
                const std::string socket_msg = Base64::Encode(msg) + DELIMETR_STR;
                this->sendBuffer.push_back(socket_msg);
            }
            catch (...)
            {
                this->logger->LogError("tcp_connection::send | Encode error");
            }
        }

        this->async_write(true);
    }

    void tcp_connection::start() {
        this->sendStart = false;
        this->partMessage = "";
        boost::asio::async_read_until(
                socket_,
                message_,
                DELIMETR,
                boost::bind(&tcp_connection::handle_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void tcp_connection::stop() {
        try {
            this->onDisconnected(this);
        } catch (...) {
        }

        this->io_service->post([this]() {
            try {
                this->socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            } catch (...) {
            }

            try {
                this->socket().close();
            } catch (...) {
            }

            try {
                this->onConnectionError.disconnect_all_slots();
                this->onConnectionSuccess.disconnect_all_slots();
                this->onConnectionTimedOut.disconnect_all_slots();
                this->onDisconnected.disconnect_all_slots();
                this->onReadError.disconnect_all_slots();
                this->onReadMessage.disconnect_all_slots();
                this->onSendError.disconnect_all_slots();
            } catch (...) {
            }
        });
    }

    void tcp_connection::disconnect() {
        this->stop();
    }
}// namespace Diginext::Core::TCP