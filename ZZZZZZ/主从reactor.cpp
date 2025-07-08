#include <iostream>
#include <thread>
#include <vector>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

const int MAX_EVENTS = 10;
const int WORKER_COUNT = 2;

int setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// 从Reactor线程类，负责处理客户端的读事件
class WorkerReactor
{
public:
    WorkerReactor()
    {
        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1)
        {
            perror("epoll_create1");
            exit(EXIT_FAILURE);
        }
    }

    void addClient(int client_fd)
    {
        epoll_event event{};
        event.data.fd = client_fd;
        event.events = EPOLLIN | EPOLLET; // 读事件，边缘触发
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
        {
            perror("epoll_ctl: add client");
            close(client_fd);
        }
    }

    void run()
    {
        epoll_event events[MAX_EVENTS];
        while (true)
        {
            int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
            if (n == -1)
            {
                perror("epoll_wait");
                break;
            }
            for (int i = 0; i < n; ++i)
            {
                if (events[i].events & EPOLLIN)
                {
                    handleRead(events[i].data.fd);
                }
            }
        }
    }

private:
    int epoll_fd;

    void handleRead(int client_fd)
    {
        char buf[512];
        while (true)
        {
            ssize_t count = read(client_fd, buf, sizeof(buf));
            if (count == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // 没有数据了
                    break;
                }
                perror("read");
                close(client_fd);
                break;
            }
            else if (count == 0)
            {
                // 客户端关闭连接
                close(client_fd);
                break;
            }
            else
            {
                // 简单回显收到的数据
                write(client_fd, buf, count);
            }
        }
    }
};

// 主Reactor类，负责监听accept事件，并把新连接分发给从Reactor
class MainReactor
{
public:
    MainReactor(int port) : port(port), next_worker(0) {}

    void start()
    {
        setupListenSocket();
        setupWorkers();

        epoll_event event{};
        event.data.fd = listen_fd;
        event.events = EPOLLIN;

        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event);

        epoll_event events[MAX_EVENTS];

        while (true)
        {
            int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
            if (n == -1)
            {
                perror("epoll_wait");
                break;
            }

            for (int i = 0; i < n; ++i)
            {
                if (events[i].data.fd == listen_fd)
                {
                    acceptNewConnections();
                }
            }
        }
    }

private:
    int port;
    int listen_fd;
    int epoll_fd;
    std::vector<std::thread> worker_threads;
    std::vector<WorkerReactor *> workers;
    int next_worker;

    void setupListenSocket()
    {
        listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd == -1)
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(listen_fd, (sockaddr *)&addr, sizeof(addr)) == -1)
        {
            perror("bind");
            exit(EXIT_FAILURE);
        }
        if (listen(listen_fd, SOMAXCONN) == -1)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        setNonBlocking(listen_fd);

        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1)
        {
            perror("epoll_create1");
            exit(EXIT_FAILURE);
        }
    }

    void setupWorkers()
    {
        for (int i = 0; i < WORKER_COUNT; ++i)
        {
            WorkerReactor *worker = new WorkerReactor();
            workers.push_back(worker);
            worker_threads.emplace_back([worker]()
                                        { worker->run(); });
        }
    }

    void acceptNewConnections()
    {
        while (true)
        {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(listen_fd, (sockaddr *)&client_addr, &client_len);
            if (client_fd == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // 没有更多连接
                    break;
                }
                else
                {
                    perror("accept");
                    break;
                }
            }
            setNonBlocking(client_fd);

            // 分发给一个从Reactor（轮询）
            workers[next_worker]->addClient(client_fd);
            next_worker = (next_worker + 1) % WORKER_COUNT;
        }
    }
};

int main()
{
    MainReactor server(8080);
    server.start();
    return 0;
}
