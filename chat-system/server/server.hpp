#pragma once


#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sstream>
#include "subreactor.hpp"


#include "../threadpool/threadpool.hpp"
#include "../log/logger.hpp"

class Server
{

public:
    Server(int port, int sub_count = 10);
    ~Server();

private:
    int listen_fd;
    int epfd;
    threadpool thread_pool;
    std::vector<SubReactor *> workers;

    Logger logger;
};


// 日志怎么处理？？？？？？？？？？？？？？