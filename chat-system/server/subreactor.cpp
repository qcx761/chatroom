#include "subreactor.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

using namespace std;

SubReactor::SubReactor(threadpool* pool) :thread_pool(pool){
    epfd = epoll_create1(0);
    running = true;
    event_thread = std::thread(&SubReactor::run, this);
    heartbeat_thread = std::thread(&SubReactor::heartbeatCheck, this);
}

SubReactor::~SubReactor() {
    running = false;
    if (event_thread.joinable()) {event_thread.join();}
    if (heartbeat_thread.joinable()){ heartbeat_thread.join();}
    close(epfd);
    for (auto& [fd, conn] : connections) {
        conn->closeConn();
        delete conn;
    }
}

void SubReactor::addClient(int client_fd) {
    // int flags = fcntl(client_fd, F_GETFL, 0);
    // fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = client_fd;

    std::lock_guard<std::mutex> lock(conn_mtx);
    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
    connections[client_fd] = new Connection(client_fd,thread_pool);
}

void SubReactor::run() {
    epoll_event events[1024];
    while (running) {
        int n = epoll_wait(epfd, events, 10, 1000);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            while (true) {
                char buf[1024];
                ssize_t len = read(fd, buf, sizeof(buf));
                if (len == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // 读完所有当前数据，退出循环等待下一次事件
                        break;
                    } else {
                        // 发生错误，关闭连接并清理
                        std::lock_guard<std::mutex> lock(conn_mtx);
                        if (connections.count(fd)) {
                            connections[fd]->closeConn();
                            delete connections[fd];
                            connections.erase(fd);
                        }
                        break;
                    }
                } else if (len == 0) {
                    // 对端关闭连接，清理资源
                    std::lock_guard<std::mutex> lock(conn_mtx);
                    if (connections.count(fd)) {
                        connections[fd]->closeConn();
                        delete connections[fd];
                        connections.erase(fd);
                    }
                    break;
                } else {
                    // 正常读取到数据，处理
                    std::lock_guard<std::mutex> lock(conn_mtx);
                    if (connections.count(fd)) {
                        connections[fd]->updateHeartbeat();
                        




                        // 函数的处理



                        
                        

        // thread_pool->enqueue([conn = connections[fd], data = std::string(buf,len)] {
                        // conn->handleMessage(data.c_str(), data.size());});



                       // 先将命令储存到缓冲区后在调用线程池
                    }
                }
            }
        }
    }
}

void SubReactor::heartbeatCheck() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::lock_guard<std::mutex> lock(conn_mtx);
        for (auto it = connections.begin(); it != connections.end(); ) {
            if (!it->second->isAlive()) {
                it->second->closeConn();
                delete it->second;
                it = connections.erase(it);
            } else {++it;}
        }
    }
}