#include "TCP/TCPClient.h"

#include <chrono>
#include <iostream>

#include <boost/make_shared.hpp>

namespace Diginext::Core::TCP
{
	using namespace std::chrono_literals;

	const size_t MAX_TRY = 30;
	const size_t MAX_TRY_ON_DELETE = 240;
	
	tcp_client::pointer tcp_client::create()
	{
		return boost::make_shared<tcp_client>();
	}

	tcp_client::tcp_client()
	{
		this->io_service = &(this->ios);
		this->started_status = false;
	}
	
	tcp_client::~tcp_client()
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

	void tcp_client::handle_tcp_connection_timeout(tcp_connection* connection, tcp::endpoint& endpoint)
	{
		if (this->tcp_conn != nullptr && this->tcp_conn->getUUID() == connection->getUUID())
			this->onConnectionTimedOut(endpoint);
	}
	
	void tcp_client::handle_tcp_connection_error(
		tcp_connection* connection, tcp::endpoint& endpoint, const boost::system::error_code& ec)
	{
		if (this->tcp_conn != nullptr && this->tcp_conn->getUUID() == connection->getUUID())
			this->onConnectionError(endpoint, ec);
	}
	
	void tcp_client::handle_tcp_connection_success(tcp_connection* connection, tcp::endpoint& endpoint)
	{
		if (this->tcp_conn != nullptr && this->tcp_conn->getUUID() == connection->getUUID())
			this->onConnectionSuccess(endpoint);
	}
	
	void tcp_client::handle_tcp_connection_disconnect(tcp_connection* connection)
	{
		if (this->tcp_conn != nullptr && this->tcp_conn->getUUID() == connection->getUUID())
		this->onDisconnected();
	}
	
	void tcp_client::handle_tcp_connection_read_message(tcp_connection* connection, std::string msg)
	{
		if (this->tcp_conn != nullptr && this->tcp_conn->getUUID() == connection->getUUID())
			this->onReadMessage(msg);
	}
	
	void tcp_client::handle_tcp_connection_read_error(
		tcp_connection* connection, const boost::system::error_code error, size_t bytes_transferred)
	{
		if (this->tcp_conn != nullptr && this->tcp_conn->getUUID() == connection->getUUID())
			this->onReadError(error, bytes_transferred);
	}
	
	void tcp_client::handle_tcp_connection_send_error(
		tcp_connection* connection, const boost::system::error_code error, size_t bytes_transferred)
	{
		if (this->tcp_conn != nullptr && this->tcp_conn->getUUID() == connection->getUUID())
			this->onSendError(error, bytes_transferred);
	}

	bool tcp_client::waitStart()
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
	
	bool tcp_client::waitStop()
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

	void tcp_client::connect(tcp::endpoint& endpoint)
	{
		this->tcp_conn = tcp_connection::create(this->ios);

		this->tcp_conn->onConnectionTimedOut.connect(boost::bind(&tcp_client::handle_tcp_connection_timeout, this, _1, _2));
		this->tcp_conn->onConnectionError.connect(boost::bind(&tcp_client::handle_tcp_connection_error, this, _1, _2, _3));
		this->tcp_conn->onConnectionSuccess.connect(boost::bind(&tcp_client::handle_tcp_connection_success, this, _1, _2));
		this->tcp_conn->onDisconnected.connect(boost::bind(&tcp_client::handle_tcp_connection_disconnect, this, _1));
		this->tcp_conn->onReadMessage.connect(boost::bind(&tcp_client::handle_tcp_connection_read_message, this, _1, _2));
		this->tcp_conn->onReadError.connect(boost::bind(&tcp_client::handle_tcp_connection_read_error, this, _1, _2, _3));
		this->tcp_conn->onSendError.connect(boost::bind(&tcp_client::handle_tcp_connection_send_error, this, _1, _2, _3));
		
		this->tcp_conn->connect(endpoint);
	}

	tcp_connection::pointer tcp_client::getConnection()
	{
		return this->tcp_conn;
	}

	void tcp_client::send(std::string msg)
	{
		if (this->getConnection() != nullptr)
		{
			this->getConnection()->send(msg);
		}
	}
	
	void tcp_client::disconnect()
	{
		if (this->getConnection() != nullptr)
		{
			this->getConnection()->disconnect();
		}
	}

	void tcp_client::start()
	{
		if (this->started())
		{
			this->stop();
		}

		this->server_thread = std::thread([this]() {
			if (this->io_service == nullptr) return;
			
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
	
	void tcp_client::stop()
	{
		if (this->started())
		{
			this->disconnect();
			this->io_service->stop();
			this->waitStop();

			if (this->server_thread.joinable())
			{
				this->server_thread.join();
			}
		}
	}

	bool tcp_client::started()
	{
		std::lock_guard<std::mutex> guard(this->status_sync);
		return this->started_status;
	}
}
