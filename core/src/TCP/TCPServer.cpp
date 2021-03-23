#include "TCP/TCPServer.h"

#include <chrono>
#include <iostream>

#include <boost/make_shared.hpp>

namespace Diginext::Core::TCP
{
	using namespace std::chrono_literals;

	const size_t MAX_TRY = 30;
	const size_t MAX_TRY_ON_DELETE = 240;

	tcp_server::pointer tcp_server::create(tcp::endpoint& endpoint)
	{
		return boost::make_shared<tcp_server>(endpoint);
	}

	tcp_server::tcp_server(tcp::endpoint& endpoint)
		: acceptor_(this->ios, endpoint)
	{
		this->started_status = false;
		this->io_service = &(this->ios);
		connections = std::list<tcp_connection::pointer>();
		start_accept();
	}

	tcp_server::~tcp_server()
	{
		try
		{
			this->stop();
			this->io_service = nullptr;
		}
		catch (...)
		{
		}
	}

	tcp::acceptor* tcp_server::getAcceptor()
	{
		return &acceptor_;
	}

	void tcp_server::start_accept()
	{
		tcp_connection::pointer new_connection = tcp_connection::create(*(this->io_service));
		acceptor_.async_accept(new_connection->socket(), boost::bind(&tcp_server::handle_accept, this, new_connection, boost::asio::placeholders::error));
	}

	void tcp_server::handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error)
	{
		if (error)
		{
			this->onAcceptError(new_connection, error);
		}

		try
		{
			std::lock_guard<std::mutex> guard(this->server_sync);
			connections.push_back(new_connection);

			new_connection->onDisconnected.connect(boost::bind(&tcp_server::handle_tcp_connection_disconnected, this, _1));
			new_connection->onReadMessage.connect(boost::bind(&tcp_server::handle_tcp_connection_read_message, this, _1, _2));
			new_connection->onReadError.connect(boost::bind(&tcp_server::handle_tcp_connection_read_error, this, _1, _2, _3));
			new_connection->onSendError.connect(boost::bind(&tcp_server::handle_tcp_connection_send_error, this, _1, _2, _3));

			new_connection->start();
			this->onAccepted(new_connection);
		}
		catch (...)
		{
		}

		start_accept();
	}

	bool tcp_server::waitStart()
	{
		for (size_t try_count = 0; try_count < MAX_TRY; try_count++)
		{
			{
				std::lock_guard<std::mutex> guard(this->status_sync);
				if (this->started_status)
				{
					return true;
				}
			}

			std::this_thread::sleep_for(1s);
		}

		return false;
	}

	bool tcp_server::waitStop()
	{
		for (size_t try_count = 0; try_count < MAX_TRY; try_count++)
		{
			{
				std::lock_guard<std::mutex> guard(this->status_sync);
				if (!(this->started_status))
				{
					return true;
				}
			}

			std::this_thread::sleep_for(1s);
		}

		return false;
	}

	void tcp_server::start()
	{
		if (this->started())
		{
			this->stop();
		}

		this->server_thread = std::thread([this]() {
			{
				std::lock_guard<std::mutex> guard(this->status_sync);
				this->started_status = true;
			}

			try
			{
				this->io_service->run();
			}
			catch (...)
			{
			}

			{
				std::lock_guard<std::mutex> guard(this->status_sync);
				this->started_status = false;
			}
		});

		this->waitStart();
	}

	void tcp_server::stop()
	{
		if (this->started())
		{
			this->disconnectAll();
			this->io_service->stop();
			this->waitStop();

			if (this->server_thread.joinable())
			{
				this->server_thread.join();
			}
		}
	}

	bool tcp_server::started()
	{
		std::lock_guard<std::mutex> guard(this->status_sync);
		return this->started_status;
	}

	tcp::endpoint tcp_server::getLocalEndpoint() const
	{
		return this->acceptor_.local_endpoint();
	}

	std::string tcp_server::getLocalAddress() const
	{
		return this->acceptor_.local_endpoint().address().to_string();
	}

	unsigned short tcp_server::getPort() const
	{
		return this->acceptor_.local_endpoint().port();
	}

	std::list<tcp_connection::pointer> tcp_server::getConnections()
	{
		std::lock_guard<std::mutex> guard(this->server_sync);
		std::list<tcp_connection::pointer> copy = std::list<tcp_connection::pointer>(this->connections);
		return copy;
	}

    std::vector<tcp_connection::pointer> tcp_server::getConnectionsVector()
    {
		auto conn_list = this->getConnections();
        std::vector<tcp_connection::pointer> conn_vector = std::vector<tcp_connection::pointer>(conn_list.begin(), conn_list.end());
		return conn_vector;
	}

	tcp_connection::pointer tcp_server::getConnectionByUUID(std::string uuid)
	{
		std::list<tcp_connection::pointer> connections = this->getConnections();
		for (auto connection : connections)
		{
			if (connection->getUUID() == uuid)
			{
				return connection;
			}
		}

		return tcp_connection::pointer();
	}

	void tcp_server::removeConnectionByUUID(std::string uuid)
	{
		{
			std::lock_guard<std::mutex> guard(this->server_sync);
			for (auto it = this->connections.begin(); it != this->connections.end(); ++it)
			{
				if (it->get()->getUUID() == uuid)
				{
					this->connections.erase(it);
					break;
				}
			}
		}

		if (this->getConnectionByUUID(uuid) != nullptr)
		{
			this->removeConnectionByUUID(uuid);
		}
	}

	void tcp_server::removeConnection(tcp_connection::pointer connection)
	{
		if (connection != nullptr)
		{
			this->removeConnectionByUUID(connection->getUUID());
		}
	}

	void tcp_server::disconnectByUUID(std::string uuid)
	{
		const auto connection = this->getConnectionByUUID(uuid);
		if (connection != nullptr)
		{
			connection->disconnect();
		}
	}
	
	void tcp_server::disconnect(tcp_connection::pointer connection)
	{
		this->disconnectByUUID(connection->getUUID());
	}
	
	void tcp_server::disconnectAll()
	{
		const auto connections = this->getConnections();
		for (const auto connection : connections)
		{
			this->disconnect(connection);
		}
	}

	void tcp_server::send(tcp_connection::pointer connection, std::string message)
	{
		if (connection != nullptr)
		{
			connection->send(message);
		}
	}

    void tcp_server::send(std::string uuid, std::string message)
    {
		const auto connection = this->getConnectionByUUID(uuid);
		if (connection != nullptr)
		{
			this->send(connection, message);
		}
	}

	void tcp_server::sendAll(std::string message)
	{
		std::list<tcp_connection::pointer> connections = this->getConnections();
		for (const auto connection : connections)
		{
			try
			{
				this->send(connection, message);
			}
			catch (...)
			{
			}
		}
	}

	//event handlers
	void tcp_server::handle_tcp_connection_disconnected(tcp_connection* connection)
	{
		const tcp_connection::pointer pointer_connection = this->getConnectionByUUID(connection->getUUID());
		if (pointer_connection != nullptr)
		{
			try
			{
				connection->onDisconnected.disconnect_all_slots();
				connection->onReadError.disconnect_all_slots();
				connection->onReadMessage.disconnect_all_slots();
				connection->onSendError.disconnect_all_slots();
			}
			catch (...)
			{
				
			}

			this->removeConnection(pointer_connection);
			this->onDisconnected(pointer_connection);
		}
	}

	void tcp_server::handle_tcp_connection_read_message(tcp_connection* connection, std::string msg)
	{
		const tcp_connection::pointer pointer_connection = this->getConnectionByUUID(connection->getUUID());
		if (pointer_connection != nullptr)
		{
			this->onReadMessage(pointer_connection, msg);
		}
	}

	void tcp_server::handle_tcp_connection_read_error(tcp_connection* connection, const boost::system::error_code error, size_t bytes_transferred)
	{
		const tcp_connection::pointer pointer_connection = this->getConnectionByUUID(connection->getUUID());
		if (pointer_connection != nullptr)
		{
			this->onReadError(pointer_connection, error, bytes_transferred);
		}
	}

	void tcp_server::handle_tcp_connection_send_error(tcp_connection* connection, const boost::system::error_code error, size_t bytes_transferred)
	{
		const tcp_connection::pointer pointer_connection = this->getConnectionByUUID(connection->getUUID());
		if (pointer_connection != nullptr)
		{
			this->onSendError(pointer_connection, error, bytes_transferred);
		}
	}
}  // namespace PP_TCP