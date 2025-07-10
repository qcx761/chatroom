#ifndef MESSAGE_HPP
#define MESSAGE_HPP

// #include <string>
// #include <thread>
// #include <atomic>
// #include <semaphore.h>
// #include <sys/epoll.h>       
// #include <sys/epoll.h>     
#include <unistd.h>        


#include "../log/logger.hpp"
#include "../threadpool/threadpool.hpp"

class Client
{

public:
    Client(std::string ip, int port);


    void epoll_thread_func();
    void user_thread_func();




    ~Client();
    void run();

private:
    int epfd;
    int sock;
    Logger logger;
};

#endif