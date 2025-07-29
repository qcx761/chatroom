#pragma once
#include <thread>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>


#include "../log/logger.hpp"
#include "../threadpool/threadpool.hpp"

using json = nlohmann::json;


class SubReactor {
public:
    SubReactor(threadpool* pool);
    ~SubReactor();

    void addClient(int client_fd);

private:
    void run();
    // void heartbeatCheck();

    threadpool *thread_pool;
    

    int epfd;
    std::thread event_thread;
    // std::thread heartbeat_thread;
    std::atomic<bool> running; // 原子
    std::mutex conn_mtx;
    // std::unordered_map<int, std::chrono::steady_clock::time_point> heartbeats;
    void closeAndRemove(int fd);
};
