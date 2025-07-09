#pragma once
#include <chrono>
#include <atomic>
#include <unistd.h>

class Connection {
public:
    explicit Connection(int fd) : fd(fd), alive(true) {
        updateHeartbeat();
    }

    void updateHeartbeat() {
        last_heartbeat = std::chrono::steady_clock::now();
    }

    bool isAlive() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - last_heartbeat).count() < 30;
    }

    int getFd() const { return fd; }
    void closeConn() { if (fd >= 0) close(fd); alive = false; }
    bool isActive() const { return alive; }

private:
    int fd;
    std::atomic<bool> alive;
    std::chrono::steady_clock::time_point last_heartbeat;
};