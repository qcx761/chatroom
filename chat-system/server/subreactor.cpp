#include "subreactor.hpp"
#include "MessageQueue.hpp"
#include"json.hpp"
#include"msg.hpp"

MessageSender sender;

using namespace std;

SubReactor::SubReactor(threadpool* pool) : thread_pool(pool){
    epfd = epoll_create1(0);
    running = true;
    event_thread = std::thread(&SubReactor::run, this);
    heartbeat_thread = std::thread(&SubReactor::heartbeatCheck, this);
}

SubReactor::~SubReactor() {
    running = false;
    if (event_thread.joinable()) {event_thread.join();}
    if (heartbeat_thread.joinable()) heartbeat_thread.join();
    close(epfd);
    {
        std::lock_guard<std::mutex> lock(conn_mtx);
        for (auto& [fd, _] : fd_buffers) {
            close(fd);
        }
        fd_buffers.clear();
    }
    {
        std::lock_guard<std::mutex> lock(hb_mutex);
        fd_heartbeat_map.clear();
    }
}

void SubReactor::heartbeatCheck() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(5)); // 每 5 秒检查一次
        auto now = std::chrono::steady_clock::now();

        std::lock_guard<std::mutex> lock(hb_mutex);
        for (auto it = fd_heartbeat_map.begin(); it != fd_heartbeat_map.end();) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count();
            if (elapsed > 30) { // 超过 30 秒未收到心跳就断开
                int fd = it->first;
                std::cout << "fd=" << fd << " 心跳超时，断开连接" << endl;
                closeAndRemove(fd);
            } else {
                ++it;
            }
        }
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
    fd_heartbeat_map[client_fd] = std::chrono::steady_clock::now();
}

void SubReactor::closeAndRemove(int fd) {
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
    {
        std::lock_guard<std::mutex> lock(hb_mutex);
        fd_heartbeat_map.erase(fd);
    }
}

void SubReactor::run() {
    epoll_event events[1024];
    while (running) {
        int n = epoll_wait(epfd, events, 1024, -1);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            if (events[i].events & EPOLLIN) {
                char buf[4096];
                    while (true) {
                        ssize_t n = recv(fd, buf, sizeof(buf), 0);
                        if (n > 0) {
                            fd_buffers[fd] += std::string(buf, n);  // 拼接数据
                        } else if (n == 0) {
                            // 对端关闭连接
                            closeAndRemove(fd);
                            return;
                        } else {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                break; // 读完数据了
                            } else {
                                // 读错误，关闭连接
                                closeAndRemove(fd);
                                return;
                            }
                        }
                    }

                    auto& buffer = fd_buffers[fd];

                    // 循环解析所有完整包
                    while (true) {
                        if (buffer.size() < 4) break;  // 长度字段不够

                        uint32_t len_net = 0;
                        memcpy(&len_net, buffer.data(), 4);
                        uint32_t len = ntohl(len_net);

                        if (len == 0 || len > 10 * 1024 * 1024) {
                            // 非法长度，关闭连接
                            closeAndRemove(fd);
                            return;
                        }

                        if (buffer.size() < 4 + len) break; // 包未完全到齐

                        // 取出 JSON 字符串
                        std::string json_str = buffer.substr(4, len);

                        json request;
                        try {
                            request = json::parse(json_str);
                        } catch (...) {
                            // 解析失败关闭连接
                            closeAndRemove(fd);
                            return;
                        }

                        // 删除已解析数据
                        buffer.erase(0, 4 + len);

                        // 解析成功处理消息
                        std::string type = request.value("type", "");
                        if (type.empty()) {
                            // 处理错误消息
                            thread_pool->enqueue([fd, request]() {
                                error_msg(fd, request);
                            });
                            continue;
                        }

                        if (type == "heartbeat") {
                            cout << "收到心跳检测来自fd=" << fd << endl;
                            std::string token = request.value("token", "");
                            bool valid = redis_key_exists(token); // 验证函数
                            if (valid) {
                                // 更新心跳或在线状态
                                refresh_online_status(token);
                            } else {
                                cout << fd << "处于未登录状态" << endl;
                            }
                            // 刷新fd时间
                            std::lock_guard<std::mutex> lock(hb_mutex);
                            fd_heartbeat_map[fd] = std::chrono::steady_clock::now();
                            continue; 
                        }

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
                        }else if(type=="show_friend_list"){
                            thread_pool->enqueue([fd, request]() {
                                show_friend_list_msg(fd,request);
                            });
                            continue;
                        }else if(type=="add_friend"){
                            thread_pool->enqueue([fd, request]() {
                            add_friend_msg(fd,request);
                            });
                            continue;
                        }else if(type=="mute_friend"){
                            thread_pool->enqueue([fd, request]() {
                                mute_friend_msg(fd,request);
                            });
                            continue;
                        }else if(type=="remove_friend"){
                            thread_pool->enqueue([fd, request]() {
                            remove_friend_msg(fd,request);
                            });
                            continue;
                        }else if(type=="unmute_friend"){
                            thread_pool->enqueue([fd, request]() {
                                unmute_friend_msg(fd,request);
                            });
                            continue;
                        }else if(type=="get_friend_requests"){
                            thread_pool->enqueue([fd, request]() {
                            get_friend_requests_msg(fd,request);
                            });
                            continue;
                        }else if(type=="handle_friend_request"){
                            thread_pool->enqueue([fd, request]() {
                            handle_friend_request_msg(fd,request);
                            });
                            continue;
                        }else if(type=="get_friend_info"){
                            thread_pool->enqueue([fd, request]() {
                            get_friend_info_msg(fd,request);
                            });
                            continue;
                        }else if(type=="send_private_message"){
                            // 放入线程池消息会乱，可加消息队列处理
                            sender.enqueue(fd, request, MsgType::PRIVATE);
                            continue;
                        }else if(type=="get_private_history"){
                            thread_pool->enqueue([fd, request]() {
                            get_private_history_msg(fd,request);
                            });
                            continue; 
                        }else if(type=="get_unread_private_messages"){
                            thread_pool->enqueue([fd, request]() {
                            get_unread_private_messages_msg(fd,request);
                            });
                            continue;
                        }else if(type=="show_group_list"){
                            thread_pool->enqueue([fd, request]() {
                            show_group_list_msg(fd,request);
                            });
                            continue;
                        }else if(type=="join_group"){
                            thread_pool->enqueue([fd, request]() {
                            join_group_msg(fd,request);
                            });
                            continue;
                        }else if(type=="quit_group"){
                            thread_pool->enqueue([fd, request]() {
                            quit_group_msg(fd,request);
                            });
                            continue;
                        }else if(type=="show_group_members"){
                            thread_pool->enqueue([fd, request]() {
                            show_group_members_msg(fd,request);
                            });
                            continue;
                        }else if(type=="create_group"){
                            thread_pool->enqueue([fd, request]() {
                            create_group_msg(fd,request);
                            });
                            continue;
                        }else if(type=="set_group_admin"){
                            thread_pool->enqueue([fd, request]() {
                            set_group_admin_msg(fd,request);
                            });
                            continue;
                        }else if(type=="remove_group_admin"){
                            thread_pool->enqueue([fd, request]() {
                            remove_group_admin_msg(fd,request);
                            });
                            continue;
                        }else if(type=="remove_group_member"){
                            thread_pool->enqueue([fd, request]() {
                            remove_group_member_msg(fd,request);
                            });
                            continue;
                        }else if(type=="add_group_member"){
                            thread_pool->enqueue([fd, request]() {
                            add_group_member_msg(fd,request);
                            });
                            continue;
                        }else if(type=="dismiss_group"){
                            thread_pool->enqueue([fd, request]() {
                            dismiss_group_msg(fd,request);
                            });
                            continue;
                        }else if(type=="get_unread_group_messages"){
                            thread_pool->enqueue([fd, request]() {
                            get_unread_group_messages_msg(fd,request);
                            });
                            continue;
                        }else if(type=="get_group_history"){
                            thread_pool->enqueue([fd, request]() {
                            get_group_history_msg(fd,request);
                            });
                            continue;
                        }else if(type=="send_group_message"){
                            // 放入消息队列
                            sender.enqueue(fd, request, MsgType::GROUP);
                            continue;
                        }else if(type=="get_group_requests"){
                            thread_pool->enqueue([fd, request]() {
                            get_group_requests_msg(fd,request);
                            });
                            continue;
                        }else if(type=="handle_group_request"){
                            thread_pool->enqueue([fd, request]() {
                            handle_group_request_msg(fd,request);
                            });
                            continue;
                        }else if(type=="send_private_file"){
                            thread_pool->enqueue([fd, request]() {
                            send_private_file_msg(fd,request);
                            });
                            continue;
                        }else if(type=="send_group_file"){
                            thread_pool->enqueue([fd, request]() {
                            send_group_file_msg(fd,request);
                            });
                            continue;
                        }else if(type=="get_file_list"){
                            thread_pool->enqueue([fd, request]() {
                            get_file_list_msg(fd,request);
                            });
                            continue;
                        }else{
                            // 处理错误逻辑
                            thread_pool->enqueue([fd, request]() {
                                error_msg(fd,request);
                            });
                        }
                    }
            } else {
                // 处理异常事件
                closeAndRemove(fd);
            }
        }
    }
}