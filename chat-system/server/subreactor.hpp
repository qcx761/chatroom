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
#include <unordered_map>
#include "../threadpool/threadpool.hpp"

using json = nlohmann::json;

class SubReactor {
public:
    SubReactor(threadpool* pool);
    void addClient(int client_fd);
    ~SubReactor();
private:
    void run();
    void heartbeatCheck();
    threadpool *thread_pool;
    std::unordered_map<int, std::string> fd_buffers;
    int epfd;
    std::thread event_thread;
    std::atomic<bool> running; // 原子
    std::mutex conn_mtx;
    std::thread heartbeat_thread;
    std::unordered_map<int, std::chrono::steady_clock::time_point> fd_heartbeat_map;
    std::mutex hb_mutex;
    void closeAndRemove(int fd);
};
