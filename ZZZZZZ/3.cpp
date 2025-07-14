std::string type = j["type"];

        if (type == "log_in_response") {
            std::string status = j["status"];
            if (status == "success") {
                current_UID = j["UID"];
                std::cout << "\n✅ 登录成功，UID: " << current_UID << std::endl;
            } else {
                std::cout << "\n❌ 登录失败：" << j["reason"] << std::endl;
            }
            sem_post(&semaphore);  // 通知主线程
        }

        else if (type == "sign_up_response") {
            std::string status = j["status"];
            if (status == "success") {
                std::cout << "\n✅ 注册成功，请登录。\n";
            } else {
                std::cout << "\n❌ 注册失败：" << j["reason"] << "\n";
            }
            sem_post(&semaphore);  // 通知主线程
        }

        // 你可以继续添加其他响应类型
        // e.g., retrieve_password_response, change_password_response, etc.

        else {
            std::cout << "\n⚠️ 收到未知消息：" << j.dump(4) << "\n";
        }






void Client::epoll_thread_func(threadpool* thread_pool){
    epoll_event events[1024];
    while(running){
        int n = epoll_wait(epfd, events, 1024, -1);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            }
            LOG_ERROR(logger, "epoll_wait failed");
            break;
        }

        for(int i=0; i < n; i++){
            json j;
            int fd = events[i].data.fd;
            uint32_t evs = events[i].events;

            if ((evs & EPOLLERR) || (evs & EPOLLHUP) || (evs & EPOLLRDHUP)) {
                cout << "服务器断开连接\n";
                running = false;
                return;
            }

            if (fd == sock) {
                int ret = receive_json(sock, j);
                std::string type = j["type"];


                if (ret == 0) {
                    



                    // if(type=="log_in"){
                    // thread_pool->enqueue([fd, request]() {
                    // sign_up_msg(fd,request);
                    // });

// 服务端发来的json好像不一样要修改逻辑


                    if(type=="sign_up"){




                    }else if(type==""){

                    }
                                        



                    // 这里处理接收到的json消息


                    // 例如：
                    // handle_server_message(j);

                    // 根据协议，处理完毕后可能需要 sem_post() 解锁等待线程

                    // sem_post(&sem);



                    
                    else{
                        std::cout << "收到未知消息";
                    }

                } else if (ret == -1) {
                    cout << "接收数据失败或服务器关闭连接\n";
                    running = false;
                    return;
                }
            } else {
                // 监听的其他fd触发了，异常处理
                LOG_ERROR(logger, "未知文件描述符事件");
                break;
                }
        }
    }
}