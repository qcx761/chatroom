#include "client.hpp"
#include "menu.hpp"
#include "account.hpp"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>

Client::Client(const std::string& ip, int port)
    : ip(ip), port(port), logger(Logger::Level::DEBUG, "client.log") {

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG_ERROR(logger, "Socket creation failed");
        exit(1);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR(logger, "Connect failed");
        exit(1);
    }

    int flags = fcntl(sock, F_GETFL);
    if (flags == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_ERROR(logger, "fcntl failed");
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

Client::~Client() {
    stop(); // 优雅关闭线程与资源
    if (sock != -1) close(sock);
    if (epfd != -1) close(epfd);
}

void Client::start() {
    running = true;
    net_thread = std::thread(&Client::epoll_thread_func, this);
    input_thread = std::thread(&Client::user_thread_func, this);
}

void Client::stop() {
    running = false;

    if (net_thread.joinable()) net_thread.join();
    if (input_thread.joinable()) input_thread.join();
}

void Client::epoll_thread_func() {
    epoll_event events[1024];

    while (running) {
        int n = epoll_wait(epfd, events, 1024, 1000);
        if (n < 0 && errno != EINTR) {
            LOG_ERROR(logger, "epoll_wait failed");
            break;
        }

        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            if (fd == sock) {
                char buf[1024] = {0};
                int count = recv(sock, buf, sizeof(buf), 0);
                if (count > 0) {
                    std::string msg(buf, count);
                    std::cout << "[Server] " << msg << std::endl;
                } else if (count == 0) {
                    std::cout << "服务器断开连接\n";
                    running = false;
                    return;
                }
            }
        }
    }
}

void Client::user_thread_func() {
    while (running) {
        main_menu_ui(sock);
        // 你可以在 main_menu_ui 中处理退出请求，例如：
        // if (user_wants_exit) running = false;
    }
}
