#include <Storage/StorageClient.h>
#include <TCP/TCP.h>

#include <chrono>
#include <iostream>
#include <string>

using namespace Diginext::Core::Storage;
using namespace std::chrono_literals;

void waitAnswer(StorageClient::pointer client, const int MAX_COUNT = 15)
{
    for (int i = 0; i < MAX_COUNT; i++)
    {
        std::string answer = client->getAnswer();
        if (answer != "") {
            std::cout << "answer received: " + answer << std::endl;
            break;
        }

        std::this_thread::sleep_for(1s);
    }
}

int main(int argc, char** argv) {
    const std::string INFO_MESSAGE = "write json request or quit for exit";
    const std::string QUIT_REQUEST = "quit";

    Diginext::Core::TCP::tcp_all_log_disable();

    while (true) {
        std::cout << INFO_MESSAGE << std::endl;

        std::string request;
        std::getline(std::cin, request);

        std::cout << std::endl;

        if (request == QUIT_REQUEST) {
            break;
        } else {
            StorageClient::pointer client = StorageClient::create();
            client->Connect();
            client->send(request);
            waitAnswer(client);
        }
    }
}