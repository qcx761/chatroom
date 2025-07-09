#pragma once
#include <thread>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <sys/epoll.h>
#include "connection.hpp"

class SubReactor {
public:
    SubReactor();
    ~SubReactor();

    void addClient(int client_fd);

private:
    void run();
    void heartbeatCheck();

    int epfd;
    std::thread event_thread;
    std::thread heartbeat_thread;
    std::atomic<bool> running;
    std::mutex conn_mtx;
    std::unordered_map<int, Connection*> connections;
};
