#pragma once
#include <chrono>
#include <atomic>
#include <unistd.h>

#include "../threadpool/threadpool.hpp"
#include "../log/logger.hpp"

// 每一个客户端连接的处理类
class Connection {
public:
    Connection(int fd, threadpool* pool);

    void updateHeartbeat();

    bool isAlive() const;

    void closeConn();

    int getFd() const;

    bool isActive() const;




    // 接口实现，怎么读取







        // void Connection::handleMessage(const char* data, size_t len) {
    // 函数的处理
    // 区分模块？？
    // }
    // ...

private:
    threadpool* thread_pool;
    int fd;
    std::atomic<bool> alive;
    std::chrono::steady_clock::time_point last_heartbeat;
};
