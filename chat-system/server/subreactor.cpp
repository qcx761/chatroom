#include "subreactor.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

SubReactor::SubReactor() {
    epfd = epoll_create1(0);
    running = true;
    event_thread = std::thread(&SubReactor::run, this);
    heartbeat_thread = std::thread(&SubReactor::heartbeatCheck, this);
}

SubReactor::~SubReactor() {
    running = false;
    if (event_thread.joinable()) event_thread.join();
    if (heartbeat_thread.joinable()) heartbeat_thread.join();
    close(epfd);
    for (auto& [fd, conn] : connections) {
        conn->closeConn();
        delete conn;
    }
}

void SubReactor::addClient(int client_fd) {
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = client_fd;

    std::lock_guard<std::mutex> lock(conn_mtx);
    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
    connections[client_fd] = new Connection(client_fd);
}

void SubReactor::run() {
    epoll_event events[10];
    while (running) {
        int n = epoll_wait(epfd, events, 10, 1000);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            char buf[1024];
            int len = read(fd, buf, sizeof(buf));
            if (len > 0) {
                std::lock_guard<std::mutex> lock(conn_mtx);
                if (connections.count(fd)) connections[fd]->updateHeartbeat();
                write(fd, buf, len); // echo
            } else {
                std::lock_guard<std::mutex> lock(conn_mtx);
                if (connections.count(fd)) {
                    connections[fd]->closeConn();
                    delete connections[fd];
                    connections.erase(fd);
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
            } else ++it;
        }
    }
}