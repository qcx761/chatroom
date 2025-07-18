#include "subreactor.hpp"
#include"json.hpp"
#include"msg.hpp"

using namespace std;

SubReactor::SubReactor(threadpool* pool) :thread_pool(pool){
    epfd = epoll_create1(0);
    running = true;
    event_thread = std::thread(&SubReactor::run, this);
    heartbeat_thread = std::thread(&SubReactor::heartbeatCheck, this);
}

SubReactor::~SubReactor() {
    // running = false;
    if (event_thread.joinable()) {event_thread.join();}
    if (heartbeat_thread.joinable()){ heartbeat_thread.join();}
    close(epfd);
    for (auto& [fd, _] : heartbeats) {
        close(fd);
    }
    heartbeats.clear();
}

void SubReactor::addClient(int client_fd) {
    // int flags = fcntl(client_fd, F_GETFL, 0);
    // fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

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
            int ret=receive_json(fd,request);

            if(ret!=0){
                std::lock_guard<std::mutex> lock(conn_mtx);
                if (heartbeats.count(fd)) {closeAndRemove(fd);}
                continue;
            }

            // 更新心跳
            {
                std::lock_guard<std::mutex> lock(conn_mtx);
                heartbeats[fd] = std::chrono::steady_clock::now();
            }

            std::string type=request.value("type","");

            if(type=="log_in"){
                thread_pool->enqueue([fd, request]() {
                    log_in_msg(fd,request);
                });
                continue;
            }else if(type=="sign_up"){
                thread_pool->enqueue([fd, request]() {
                    sign_up_msg(fd,request);
                });
                continue;
            }
            
// 每次函数之前都验证就好了
//             // 校验 token
//             std::string token = request.value("token", "");
//             std::string username;
//             if (token.empty() || !verify_token(token, username)) {
// // 去掉redis中的缓存来消除上线状态
//                 json resp;
//                 resp["type"] = type;
//                 resp["status"] = "error";
//                 resp["msg"] = "Invalid or expired token.";
//                 send_json(fd, resp);
//                 continue;
//             }
            
            
            if(type=="destory_account"){
                thread_pool->enqueue([fd, request]() {
                    destory_account_msg(fd,request);
                });
                continue;
            }else if(type=="quit_account"){
                thread_pool->enqueue([fd, request]() {
                    quit_account_msg(fd,request);
                });
                continue;
            }else if(type=="username_view"){
                thread_pool->enqueue([fd, request]() {
                    username_view_msg(fd,request);
                });
                continue;
            }else if(type=="username_change"){
                thread_pool->enqueue([fd, request]() {
                    username_change_msg(fd,request);
                });
                continue;
            }else if(type=="password_change"){
                thread_pool->enqueue([fd, request]() {
                    password_change_msg(fd,request);
                });
                continue;
            }else if(type==""){






                continue;

            }else if(type==""){




                continue;

            }else if(type==""){





                continue;

            }else if(type==""){





                


                continue;

            }else if(type==""){





                

                continue;


            }else if(type==""){





                


                continue;

            }else{
                // 处理错误逻辑
                thread_pool->enqueue([fd, request]() {
                    error_msg(fd,request);
                });
            }
        }
    }
}

void SubReactor::heartbeatCheck() {
    while (running) {


// 服务端要发送心跳检测包



        std::this_thread::sleep_for(std::chrono::seconds(1000000000));
        auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(conn_mtx);
        for (auto it = heartbeats.begin(); it != heartbeats.end(); ) {
            if (std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count() > 10) {
                std::cout << "客户端超时断开: fd = " << it->first << "\n";
                close(it->first);
                epoll_ctl(epfd, EPOLL_CTL_DEL, it->first, nullptr);
                it = heartbeats.erase(it);
            } else {
                ++it;
            }
        }
    }
}