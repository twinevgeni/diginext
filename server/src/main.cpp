#include <Storage/StorageServer.h>
#include <TCP/TCP.h>

#include <chrono>

using namespace Diginext::Core::Storage;
using namespace std::chrono_literals;

int main(int argc, char** argv) {
    Diginext::Core::TCP::tcp_all_log_disable();

    StorageServer::pointer server = StorageServer::create();
    server->Start();

    while (server->Started())
    {
        std::this_thread::sleep_for(1s);
    }
}