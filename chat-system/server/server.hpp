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

class Server
{

public:
    Server(int Port);
    ~Server();

private:
    
int listen_fd;
int port;
int epfd;


};

#endif