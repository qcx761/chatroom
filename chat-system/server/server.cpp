#include "server.hpp"
// #include "../log/logger.hpp"
// #include "../threadpool/threadpool.hpp"

using namespace std;

#define MAX_EVENTS 10
#define thread_count 10

// 记录等级为 DEBUG 及以上的所有日志”写入 client.log 文件中
Server::Server(int port, int sub_count) : thread_pool(thread_count), logger(Logger::Level::DEBUG, "server.log")
{
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        LOG_ERROR(logger, "socket failed");
        exit(1);
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(listen_fd, (sockaddr *)&addr, addr_len) < 0)
    {
        LOG_ERROR(logger, "bind failed");
        close(listen_fd);
        exit(1);
    }

    if (listen(listen_fd, 10) < 0)
    {
        LOG_ERROR(logger, "listen failed");

        close(listen_fd);
        exit(1);
    }

    epfd = epoll_create1(0);
    if (epfd == -1)
    {
        LOG_ERROR(logger, "epoll_create failed");
        close(listen_fd);
        exit(1);
    }

    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev) == -1)
    {
        LOG_ERROR(logger, "epoll_ctl ADD server_fd failed");
        close(listen_fd);
        close(epfd);
        exit(1);
    }

    // 创建 sub_count 个 SubReactor 对象（也就是从 Reactor 线程或者事件循环实例），并将它们的指针依次存入 workers 容器（std::vector<SubReactor*>）
    for (int i = 0; i < sub_count; ++i)
        workers.push_back(new SubReactor(&thread_pool));

    int idx = 0;
    epoll_event events[MAX_EVENTS];
    while (1)
    {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (n == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            LOG_ERROR(logger, "epoll_wait failed");
            break;
        }
        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;
            uint32_t evs = events[i].events;

            if ((evs & EPOLLERR) || (evs & EPOLLHUP) || (evs & EPOLLRDHUP))
            {
                close(fd);
                continue;
            }

            if (fd == listen_fd)
            { // 客户端连接
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(listen_fd, (sockaddr *)&client_addr, &client_len);

                if (client_fd == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        LOG_ERROR(logger, "client_accpet failed");
                        break;
                    }
                }

                // set_nonblocking(client_fd);
                int flags = fcntl(client_fd, F_GETFL, 0);
                if (flags == -1)
                {
                    LOG_ERROR(logger, "fcntl F_GETFL failed");

                    return;
                }
                if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1)
                {
                    LOG_ERROR(logger, "fcntl F_SETFL failed");
                }

                workers[idx]->addClient(client_fd);
                idx = (idx + 1) % sub_count;

                //     epoll_event client_ev{};
                //     client_ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLERR;
                //     client_ev.data.fd = client_fd;

                //     if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &client_ev) == -1)
                //     {
                //         LOG_ERROR(logger, "epoll_ctl ADD client_fd failed");
                //         close(client_fd);
                //     }

                //     std::ostringstream oss;
                //     oss << "New control connection accepted: fd=" << client_fd;

                //     LOG_INFO(logger, oss.str());
                // }
                // else if (evs & EPOLLOUT)
                // {
                //     // 执行客户端的要求
                // }
                // else
                // {
                // }
            }
        }
    }
}

Server::~Server()
{
    for (auto sub : workers)
    {
        delete sub;
    }
    close(listen_fd);
    close(epfd);
}