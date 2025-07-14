#include "connection.hpp"
#include <chrono>
#include <unistd.h> // close()

Connection::Connection(int fd, threadpool* pool) : fd(fd), alive(true), thread_pool(pool) {
    updateHeartbeat();
}

void Connection::updateHeartbeat() {
    last_heartbeat = std::chrono::steady_clock::now();
}

bool Connection::isAlive() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - last_heartbeat).count() < 30;
}

void Connection::closeConn() {
    if (fd >= 0) close(fd);
    alive = false;
}

int Connection::getFd() const {
    return fd;
}

bool Connection::isActive() const {
    return alive.load();  // atomic 读要用load()
}
