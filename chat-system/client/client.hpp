#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <regex.h>
#include <openssl/ssl.h>
#include <termios.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include "../log/logger.hpp"
#include "../threadpool/threadpool.hpp"

class Client
{

public:
    Client(std::string ip, int port);
    ~Client();

private:
    int sock;
    Logger logger;
};

#endif