#include <Storage/StorageClient.h>
#include <TCP/TCP.h>

#include <chrono>
#include <iostream>
#include <string>

using namespace Diginext::Core::Storage;
using namespace std::chrono_literals;

int main(int argc, char** argv) {
    const std::string INFO_MESSAGE = "write json request or quit for exit";
    const std::string QUIT_REQUEST = "quit";

    Diginext::Core::TCP::tcp_all_log_disable();

    while (true) {
        std::cout << INFO_MESSAGE << std::endl;

        std::string request;
        std::cin >> request;

        std::cout << std::endl;

        if (request == QUIT_REQUEST) {
            break;
        } else {
            StorageClient::pointer client = StorageClient::create();
            client->Connect();
            client->send(request);
            std::this_thread::sleep_for(5s);
        }
    }
}