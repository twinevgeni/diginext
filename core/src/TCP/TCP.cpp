#include "TCP/TCP.h"

namespace Diginext::Core::TCP
{
	bool _tcp_connection_log_enabled = true;
	bool _tcp_client_log_enabled = true;
	bool _tcp_server_log_enabled = true;

	bool tcp_connection_log_enabled()
	{
		return _tcp_connection_log_enabled;
	}

	bool tcp_client_log_enabled()
	{
		return _tcp_client_log_enabled;
	}

	bool tcp_server_log_enabled()
	{
		return _tcp_server_log_enabled;
	}

	void tcp_all_log_enable()
	{
		_tcp_connection_log_enabled = true;
		_tcp_client_log_enabled = true;
		_tcp_server_log_enabled = true;
	}

	void tcp_all_log_disable()
	{
		_tcp_connection_log_enabled = false;
		_tcp_client_log_enabled = false;
		_tcp_server_log_enabled = false;
	}

}  // namespace PP_TCP