#include "client.hpp"
using namespace std;

Client::Client(std::string ip, int port) : logger(Logger::Level::DEBUG, "client.log")
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        LOG_ERROR(logger, "socket failed");
        exit(1);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if (connect(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        LOG_ERROR(logger, "connect failed");
        exit(1);
    }
}

Client::~Client()
{
    close(sock);
}