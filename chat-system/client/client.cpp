#include"client.hpp" // 客户端
#include"menu.hpp"  // 目录
#include"json.hpp"  // json发送
#include"account.hpp"// 阻塞函数
#include"msg.hpp"  // 信息处理函数
using namespace std;

Client::Client(std::string ip, int port) :thread_pool(10), logger(Logger::Level::DEBUG, "client.log")
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        LOG_ERROR(logger, "socket failed");
        exit(1);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if (connect(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        LOG_ERROR(logger, "connect failed");
        exit(1);
    }


    int flags=fcntl(sock, F_GETFL) ;
    if (flags == -1) {
        LOG_ERROR(logger, "fcntl F_GETFL failed");

        exit(1);
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_ERROR(logger, "fcntl F_SETFL failed");

        exit(1);
    }
    epfd = epoll_create1(0);
        if (epfd == -1) {
        LOG_ERROR(logger, "epoll_create1 failed");
        exit(1);
    }
    epoll_event ev{};
    ev.data.fd = sock;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);


}

Client::~Client()
{
    stop();
    if (sock != -1) close(sock);
    if (epfd != -1) close(epfd);

    sem_destroy(&sem);  // 如果你用了信号量
}

void Client::start() {
    running = true;

    sem_init(&sem, 0, 0);  // 初始化信号量  
    
    // 启动 epoll 网络线程
    net_thread = std::thread(&Client::epoll_thread_func, this);
    // 启动用户输入线程
    input_thread = std::thread(&Client::user_thread_func, this);
}

void Client::stop() {
    // running = false;

    if (net_thread.joinable()) net_thread.join();
    if (input_thread.joinable()) input_thread.join();
}

void Client::epoll_thread_func(){
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
                // 接收服务端发送的信息，并且解放阻塞线程的信号量
                


                if (ret == 0) {

                    // 超时登录处理
                    if(j["msg"]=="Invalid or expired token"){
                        cout <<"登录超时请重新登录。"<<endl;
                        login_success.store(false);  // 判断登录
                        token.clear();
                        state=main_menu;
                        //waiting();
                        sem_post(&sem);  
                        continue;
                    }
                    
                    if(type=="sign_up"){
                        thread_pool.enqueue([this,j]() {
                            sign_up_msg(j);
                            sem_post(&this->sem);  // 通过 this 访问成员变量
                        });
                        continue;
                    }

                    if(type=="log_in"){
                        thread_pool.enqueue([this,j]() {
                            bool success_if = log_in_msg(j,this->token);
                            this->login_success.store(success_if);
                            sem_post(&this->sem);
                        });
                        continue;
                    }

                    if(type=="destory_account"){
                        thread_pool.enqueue([this,j]() {
                            destory_account_msg(j);
                            state=main_menu;
                            login_success.store(false);
                            token.clear();
                            sem_post(&this->sem);
                        });         
                        continue;
                    }

                    if(type=="quit_account"){
                        thread_pool.enqueue([this,j]() {
                            quit_account_msg(j);
                            state=main_menu;
                            login_success.store(false);
                            token.clear();
                            sem_post(&this->sem);
                        });
                        continue;
                    }

                    if(type=="username_view"){
                        thread_pool.enqueue([this,j]() {
                            username_view_msg(j);
                            sem_post(&this->sem);        
                        });
                        continue;
                    }

                    if(type=="username_change"){
                        thread_pool.enqueue([this, j]() {
                            username_change_msg(j);
                            sem_post(&this->sem);
                        });
                        continue;
                    }

                    if(type=="password_change"){
                        thread_pool.enqueue([this,j]() {
                            password_change_msg(j);
                            sem_post(&this->sem);
                        });
                        continue;
                    }

                    if(type=="show_friend_list"){
                        thread_pool.enqueue([this,j]() {
                            show_friend_list_msg(j);
                            sem_post(&this->sem);
                        });
                        continue;
                    }    
                    
                    if(type=="add_friend"){
                        thread_pool.enqueue([this,j]() {
                            add_friend_msg(j);
                            sem_post(&this->sem);
                        });
                        continue;
                    } 

                    if(type=="remove_friend"){
                        thread_pool.enqueue([this,j]() {
                            remove_friend_msg(j);
                            sem_post(&this->sem);
                        });
                        continue;
                    } 

                    if(type=="mute_friend"){
                        thread_pool.enqueue([this,j]() {
                            mute_friend_msg(j);
                            sem_post(&this->sem);
                        });
                        continue;
                    } 

                    if(type=="unmute_friend"){
                        thread_pool.enqueue([this,j]() {
                            unmute_friend_msg(j);
                            sem_post(&this->sem);
                        });
                        continue;
                    }

                    if(type=="get_friend_requests"){
                        thread_pool.enqueue([this,j]() {
                            get_friend_requests_msg(j);
                            sem_post(&this->sem);
                        });
                        continue;
                    }


                    if(type=="handle_friend_request"){
                        thread_pool.enqueue([this,j]() {
                            handle_friend_request_msg(j);
                            sem_post(&this->sem);
                        });
                        continue;
                    }

                    if(type=="get_friend_info"){
                        thread_pool.enqueue([this,j]() {
                            get_friend_info_msg(j);
                            sem_post(&this->sem);
                        });
                        continue;
                    }
                





























                    if(type=="show_friend_notifications"){
                        thread_pool.enqueue([this,j]() {
                            show_friend_notifications_msg(j);
                            sem_post(&this->sem);
                        });
// 信号处理后面还要添加
                        continue;
                    }

                    if(type=="get_private_history"){
                        thread_pool.enqueue([this,j]() {
                            get_private_history_msg(j);
                            sem_post(&this->sem);
                        });
                        continue;
                    }

                    if(type=="send_private_message"){
                        thread_pool.enqueue([this,j]() {
                            send_private_message_msg(j);
                            sem_post(&this->sem);
                        });
                        continue;
                    }

                    if(type=="receive_private_message"){
                        thread_pool.enqueue([this,j]() {
                            receive_private_message_msg(j);
                            // sem_post(&this->sem);
                        });
                        continue;
                    }























                    if(type==""){
                        thread_pool.enqueue([this,j]() {
                            //_msg(j);
                            //state=main_menu;
                            //login_success.store(false);
                            //token.clear();
                            sem_post(&this->sem);
                        });
                        continue;
                    }

                    if(type==""){
                        thread_pool.enqueue([this,j]() {
                            //_msg(j);
                            //state=main_menu;
                            //login_success.store(false);
                            //token.clear();
                            sem_post(&this->sem);
                        });
                        continue;
                    }

                    if(type==""){
                        thread_pool.enqueue([this,j]() {
                            //_msg(j);
                            //state=main_menu;
                            //login_success.store(false);
                            //token.clear();
                            sem_post(&this->sem);
                        });
                        continue;
                    }

                    if(type==""){
                        thread_pool.enqueue([this,j]() {
                            //_msg(j);
                            //state=main_menu;
                            //login_success.store(false);
                            //token.clear();
                            sem_post(&this->sem);
                        });
                        continue;
                    }


                    

























                    if(type=="error"){
                    // 处理错误信息
                    }

                    




                    std::cout << "收到未知消息";
                } else if (ret == -1) {
                    cout << "接收数据失败或服务器关闭连接\n";
                    running = false;
                    return;
                }else{
                    // 不会进入
                    return;
                }


            } else {
                // 监听的其他fd触发了，异常处理,一般不会进入
                LOG_ERROR(logger, "未知文件描述符事件");
                break;
            }
        }
    }
}

void Client::user_thread_func() {

    state = main_menu; // 初始化界面
    while(running){

        // 登录过期检测
        if (!login_success.load() && state != main_menu) {
            std::cout << "请重新登录。" << std::endl;
            token.clear();                   // 清除token
            state = main_menu;               // 返回登录页
            waiting();                       // 等待用户确认
            continue;
        }

        // 主交互逻辑
        switch(state)
        {
            // 对应menu.cpp查看
            case main_menu:
            {
                main_menu_ui(sock,sem,login_success);
                state=next_menu; // 进入主界面
                break;
            }

            case next_menu:
            {
                int m;
                show_next_menu();
                if (!(cin >> m)) {
                    flushInput();
                    cout << "无效的输入，请输入数字。" << endl;
                    waiting();
                    continue;
                }
                switch (m)
                {
                case 1: state=next1_menu; break; // 进入个人中心
                case 2: state=next2_menu; break;




                case 3: 
                case 4: state=next4_menu; break;




                default:
                    cout << "无效数字" << endl;
                    flushInput(); // 去除数字后面的换行符
                    waiting();
                }
                break;
            }

            case next1_menu:
            {
                int m;
                show_next1_menu();
                if (!(cin >> m)) {
                    flushInput();
                    cout << "无效的输入，请输入数字。" << endl;
                    waiting();
                    continue;
                }
                switch (m)
                {
                case 1: state=next11_menu; break; 
                // 注销账户要把所有东西清除
                case 2: destory_account(sock,token,sem); break;
                case 3: quit_account(sock,token,sem); break;
                case 4: state=next_menu; break;
                default:
                    cout << "无效数字" << endl;
                    flushInput(); // 去除数字后面的换行符
                    waiting();
                }
                break;
            }

            case next11_menu:
            {
                int m;
                show_next11_menu();
                if (!(cin >> m)) {
                    flushInput();
                    cout << "无效的输入，请输入数字。" << endl;
                    waiting();
                    continue;
                }
                switch (m)
                {
                case 1: username_view(sock,token,sem); break;
                case 2: username_change(sock,token,sem); break;
                case 3: password_change(sock,token,sem); break;
                case 4: state=next1_menu; break;
                default:
                    cout << "无效数字" << endl;
                    flushInput(); // 去除数字后面的换行符
                    waiting();
                }
                break;
            }

            case next2_menu:
            {
                int m;
                show_next2_menu();
                if (!(cin >> m)) {
                    flushInput();
                    cout << "无效的输入，请输入数字。" << endl;
                    waiting();
                    continue;
                }
                switch (m)
                {
                case 1: state=next21_menu; break;
                case 2: state=next22_menu; break;
                case 3: getandhandle_friend_request(sock,token,sem); break;
                case 4: 
                case 5: 
                case 6: state=next_menu; break;
                case 7: 
                case 8: 
                default:
                    cout << "无效数字" << endl;
                    flushInput(); // 去除数字后面的换行符
                    waiting();
                }
                break;
            }

            case next21_menu:
            {
                int m;
                show_next21_menu();
                if (!(cin >> m)) {
                    flushInput();
                    cout << "无效的输入，请输入数字。" << endl;
                    waiting();
                    continue;
                }
                switch (m)
                {
                case 1: show_friend_list(sock,token,sem); break;
                case 2: add_friend(sock,token,sem); break;
                case 3: remove_friend(sock,token,sem); break;
                case 4: send_private_message(sock,token,sem); break;
                case 5: unmute_friend(sock,token,sem); break;
                case 6: mute_friend(sock,token,sem); break;
                case 7: 
                case 8: get_friend_info(sock,token,sem); break;
                case 9: state=next2_menu; break;
                default:
                    cout << "无效数字" << endl;
                    flushInput(); // 去除数字后面的换行符
                    waiting();
                }
                break;
            }

            case next22_menu:
            {
                int m;
                show_next22_menu();
                if (!(cin >> m)) {
                    flushInput();
                    cout << "无效的输入，请输入数字。" << endl;
                    waiting();
                    continue;
                }
                switch (m)
                {
                case 1: 
                case 2: 
                case 3: 
                case 4: 
                case 5: 
                case 6: 
                case 7: 
                case 8: 
                case 9: 
                case 10: 
                case 11: 
                case 12: 
                case 13: state=next2_menu; break;
                default:
                    cout << "无效数字" << endl;
                    flushInput(); // 去除数字后面的换行符
                    waiting();
                }
                break;
            }

            case next4_menu:
            {
                int m;
                show_next4_menu();
                if (!(cin >> m)) {
                    flushInput();
                    cout << "无效的输入，请输入数字。" << endl;
                    waiting();
                    continue;
                }
                switch (m)
                {
                case 1: show_friend_notifications(sock,token,sem); break;
                case 2: 
                case 3: 
                case 4: state=next_menu; break;
                default:
                    cout << "无效数字" << endl;
                    flushInput(); // 去除数字后面的换行符
                    waiting();
                }
                break;
            }

            // case next1_menu:
            // {
            //     int m;
            //     //show_next_menu();
            //     if (!(cin >> m)) {
            //         flushInput();
            //         cout << "无效的输入，请输入数字。" << endl;
            //         waiting();
            //         continue;
            //     }
            //     switch (m)
            //     {
            //     case 1: 
            //     case 2: 
            //     case 3: 
            //     case 4: 
            //     }
            //     break;
            // }






        } // switch处理
    } // while循环
} // 线程





