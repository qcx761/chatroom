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
                        




                        
                cout<<"111111111111111111111111111111"<<endl;

                        // 函数的处理



                        
                        

        // thread_pool->enqueue([conn = connections[fd], data = std::string(buf,len)] {
                        // conn->handleMessage(data.c_str(), data.size());});



                       // 先将命令储存到缓冲区后在调用线程池
                    }
                }
            }



void SubReactor::run() {
    epoll_event events[1024];
    while (running) {
        int n = epoll_wait(epfd, events, 1024, 1000);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;

            json request;
            int ret = receive_json(fd, request);

            if (ret != 0) {
                // 读取失败或连接关闭，清理连接
                std::lock_guard<std::mutex> lock(conn_mtx);
                if (connections.count(fd)) {
                    connections[fd]->closeConn();
                    delete connections[fd];
                    connections.erase(fd);
                }
                continue;
            }

            // 更新心跳
            {
                std::lock_guard<std::mutex> lock(conn_mtx);
                if (connections.count(fd)) {
                    connections[fd]->updateHeartbeat();
                }
            }

            std::string type = request.value("type", "");
            json response;

            if (type == "log_in") {
                std::string username = request.value("username", "");
                std::string password = request.value("password", "");

                // 假设 check_user_login 是你实现的 Redis 验证函数
                bool login_ok = check_user_login(username, password);

                response["type"] = "log_in_response";
                response["success"] = login_ok;

                if (login_ok) {
                    std::string uid = get_uid_from_username(username);  // 你也要实现这个函数
                    response["UID"] = uid;
                } else {
                    response["error"] = "用户名或密码错误";
                }
                send_json(fd, response);
            }
            else if (type == "sign_up") {
                // 处理注册逻辑
            }
            else if (type == "retrieve_password") {
                // 处理找回密码逻辑
            }
            else {
                response["type"] = "error";
                response["msg"] = "未知请求类型: " + type;
                send_json(fd, response);
            }
        }
    }
}












#pragma once

#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <iostream>
#include "json.hpp"
#include "../threadpool/threadpool.hpp"

using json = nlohmann::json;

class SubReactor {
public:
    SubReactor(threadpool* pool);
    ~SubReactor();

    void addClient(int client_fd);

private:
    int epfd;
    std::atomic<bool> running;
    std::thread event_thread;
    std::thread heartbeat_thread;
    threadpool* thread_pool;

    std::unordered_map<int, std::chrono::steady_clock::time_point> heartbeats;
    std::mutex conn_mtx;

    void run();
    void heartbeatCheck();
    void closeAndRemove(int fd);
    int receive_json(int sockfd, json& j);
    int send_json(int sockfd, const json& j);
};


#include "subreactor.hpp"

SubReactor::SubReactor(threadpool* pool) : thread_pool(pool) {
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
    for (auto& [fd, _] : heartbeats) {
        close(fd);
    }
    heartbeats.clear();
}

void SubReactor::addClient(int client_fd) {
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = client_fd;

    std::lock_guard<std::mutex> lock(conn_mtx);
    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
    heartbeats[client_fd] = std::chrono::steady_clock::now();
}

void SubReactor::closeAndRemove(int fd) {
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
    heartbeats.erase(fd);
}

void SubReactor::run() {
    epoll_event events[1024];
    while (running) {
        int n = epoll_wait(epfd, events, 1024, 1000);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;

            json request;
            if (receive_json(fd, request) != 0) {
                std::lock_guard<std::mutex> lock(conn_mtx);
                if (heartbeats.count(fd)) closeAndRemove(fd);
                continue;
            }

            {
                std::lock_guard<std::mutex> lock(conn_mtx);
                heartbeats[fd] = std::chrono::steady_clock::now();
            }

            std::string type = request.value("type", "");
            json response;

            if (type == "log_in") {
                std::string user = request.value("username", "");
                std::string pass = request.value("password", "");
                // 示例逻辑：登录成功
                response["type"] = "log_in";
                response["status"] = "success";
                response["message"] = "登录成功";
                send_json(fd, response);
            }
            else if (type == "sign_up") {
                response["type"] = "sign_up";
                response["status"] = "success";
                response["message"] = "注册成功";
                send_json(fd, response);
            }
            else {
                response["type"] = "error";
                response["message"] = "未知请求类型";
                send_json(fd, response);
            }
        }
    }
}

void SubReactor::heartbeatCheck() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(conn_mtx);
        for (auto it = heartbeats.begin(); it != heartbeats.end(); ) {
            if (std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count() > 10) {
                std::cout << "⏱️ 客户端超时断开: fd = " << it->first << "\n";
                close(it->first);
                epoll_ctl(epfd, EPOLL_CTL_DEL, it->first, nullptr);
                it = heartbeats.erase(it);
            } else {
                ++it;
            }
        }
    }
}



