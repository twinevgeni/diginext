#ifndef DIGINEXT_CORE___TCP_TCP_H
#define DIGINEXT_CORE___TCP_TCP_H

#include <string>

namespace Diginext::Core::TCP {
    const unsigned short RANDOM_PORT = 0;
    const unsigned short DEFAULT_PORT = 9911;

    const std::string LOCAL_ADDRESS_TCP_V4 = "127.0.0.1";
    const std::string LOCAL_ADDRESS_TCP_V6 = "::1";

    bool tcp_connection_log_enabled();
    bool tcp_client_log_enabled();
    bool tcp_server_log_enabled();

    void tcp_all_log_enable();
    void tcp_all_log_disable();
}// namespace Diginext::Core::TCP

#endif//DIGINEXT_TCP_H
