#include"connection.hpp"



    Connection(int fd,threadpool* pool) : fd(fd), alive(true) {
        updateHeartbeat();
    }

    void updateHeartbeat() {
        last_heartbeat = std::chrono::steady_clock::now();
    }

    bool isAlive(){
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - last_heartbeat).count() < 30;
    }

   


    void closeConn() { if (fd >= 0) close(fd); alive = false; }


    int getFd() const { return fd; }


    bool isActive() const { return alive; }