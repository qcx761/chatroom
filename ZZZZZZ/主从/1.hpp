#ifndef MESSAGE_HPP
#define MESSAGE_HPP

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
#include<sstream>

#include "../threadpool/threadpool.hpp"
#include "../log/logger.hpp"


#define MAX_EVENTS 10
#define thread_count 10

class Server
{

public:
    Server(int port);
    ~Server();

private:
    
int listen_fd;
int epfd;
threadpool thread_pool;
Logger logger;

static void set_nonblocking(int fd);


};



#endif