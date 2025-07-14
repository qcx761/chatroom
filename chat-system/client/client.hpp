#pragma once

#include <sys/socket.h>   
#include <netinet/in.h>  
#include <arpa/inet.h>    
#include <fcntl.h>        
#include <unistd.h>       
#include <cstring>      
#include <errno.h>        
#include <string>
#include <thread>
#include <atomic>
#include <semaphore.h>
#include <sys/epoll.h>       
#include <sys/epoll.h>     
#include <unistd.h>        


#include "../log/logger.hpp"
#include "../threadpool/threadpool.hpp"

class Client
{

public:
    Client(std::string ip, int port);
    ~Client();
    void start();  // 启动客户端（含网络线程、输入线程）
    void stop();   // 请求退出并等待线程关闭


// 禁用拷贝构造
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

// 启用移动构造
    Client(Client&&) = default;
    Client& operator=(Client&&) = default;


private:
    int epfd=-1;
    int sock=-1;
    int port;
    Logger logger;
    std::atomic<bool> running = true;
    std::thread input_thread;
    std::thread net_thread;
    threadpool thread_pool;

    void epoll_thread_func(threadpool* thread_pool);
    void user_thread_func(threadpool* thread_pool);

    // void epoll_thread_func();   // 网络线程
    // void user_thread_func();    // 用户交互线程
};
