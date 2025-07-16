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

    enum MenuState {
        main_menu,    // 主菜单
        next_menu,    // 登录后主界面
        next1_menu,   // 个人中心
        next11_menu,  // 个人信息  






    };


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



    // void wait_sem() { sem_wait(&sem); }
    // void post_sem() { sem_post(&sem); }
    


private:
    int epfd=-1;
    int sock=-1;
    int port;
    Logger logger;
    std::atomic<bool> running = true;
    std::thread input_thread;
    std::thread net_thread;

    threadpool thread_pool;
    sem_t sem;                      // 信号量，用来同步等待服务器响应
    std::atomic<bool> login_success{false};  // 服务器返回的登录结果

        void epoll_thread_func();   // 网络线程
        void user_thread_func();    // 用户交互线程
};
