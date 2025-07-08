#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <functional>

int setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// 简单回调类型，事件发生时调用
using Callback = std::function<void(int)>;

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listen_fd, (sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 5);
    setNonBlocking(listen_fd);

    int epoll_fd = epoll_create1(0);
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);

    std::unordered_map<int, Callback> callbacks;

    // 主 Reactor 回调：处理新连接
    callbacks[listen_fd] = [&](int fd) {
        while (true) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(fd, (sockaddr*)&client_addr, &client_len);
            if (client_fd == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                else {
                    perror("accept");
                    break;
                }
            }
            setNonBlocking(client_fd);

            // 给新连接注册读事件
            epoll_event client_ev{};
            client_ev.events = EPOLLIN | EPOLLET;  // 边缘触发
            client_ev.data.fd = client_fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);

            // 从 Reactor 回调：处理读事件
            callbacks[client_fd] = [&](int client_fd) {
                char buf[512];
                while (true) {
                    ssize_t n = read(client_fd, buf, sizeof(buf));
                    if (n == 0) { // 连接关闭
                        close(client_fd);
                        callbacks.erase(client_fd);
                        break;
                    }
                    if (n < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        perror("read");
                        close(client_fd);
                        callbacks.erase(client_fd);
                        break;
                    }
                    write(client_fd, buf, n); // echo
                }
            };
        }
    };

    epoll_event events[10];

    while (true) {
        int n = epoll_wait(epoll_fd, events, 10, -1);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            if (callbacks.count(fd)) {
                callbacks[fd](fd);  // 事件触发，回调函数被调用
            }
        }
    }

    close(listen_fd);
    close(epoll_fd);
    return 0;
}

