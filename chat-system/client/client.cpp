#include "client.hpp"
#include"menu.hpp"
#include"account.hpp"
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


    int flags=fcntl(sock, F_GETFL) ;
    if (flags == -1) {
        LOG_ERROR(logger, "fcntl F_GETFL failed");

        exit(1);
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_ERROR(logger, "fcntl F_SETFL failed");

        exit(1);
    }
    epfd = epoll_create1(0);
        if (epfd == -1) {
        LOG_ERROR(logger, "epoll_create1 failed");
        exit(1);
    }
    epoll_event ev{};
    ev.data.fd = sock;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);


}

Client::~Client()
{
    close(sock);
}


void epoll_thread_func(){
    epoll_event events[1024];
    while(1){
            int n = epoll_wait(epfd, events, 1024, -1);
            if (n == -1) {
                if (errno == EINTR) {
                    continue;
                }
        LOG_ERROR(logger, "epoll_wait failed");

                break;
            }
        for(int i=0;i<n;i++){
            int fd=events[i].data.fd;
            uint32_t evs = events[i].events;
            if ((evs & EPOLLERR) || (evs & EPOLLHUP) || (evs & EPOLLRDHUP)) {
                break;
            }
            if(fd==sock){
                char buf[1024] = {0};
                int n = recv(sock, buf, sizeof(buf), 0);
                if (n > 0) {
                    handle_server_message(string(buf, n));
                } else if (n == 0) {
                    cout << "服务器断开连接\n";
                    exit(0);
                }





                // 处理服务端发送过来的信息

                // 只处理 recv 和 sem_post()，不做任何 cin









        }else{
            // 唯一监听的没有触发
            break;
        }

    }

}

}

void user_thread_func() {
    main_menu_ui(sockfd);
}




void client::run(){

    
    // 启动 epoll 网络线程
    thread net_thread(epoll_thread_func);

    // 启动用户输入线程
    thread input_thread(user_thread_func);

    // 等待线程结束
    input_thread.join();
    running = false;
    close(sockfd);
    net_thread.join();

    sem_destroy(&sem);



}
