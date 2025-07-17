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


        // 超时登录记得测试一下可能有点问题

                    // 超时登录处理
                    if(j["msg"]=="Invalid or expired token."){
                        cout <<"登录超时请重新登录。"<<endl;
                        waiting();
                        login_success.store(false);  // 判断登录
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
                            sem_post(&this->sem);  // 通过 this 访问成员变量
                        });
                        continue;
                    }







                    if(type=="destory_account"){
                        thread_pool.enqueue([this, fd, j]() {
                            destory_account_msg(fd, j,this->token);
                            
                            sem_post(&this->sem);  // 通过 this 访问成员变量
                        });



                        // 帐号密码错误
                        // cout<<"成功注销"<<endl;
                        
                        // 成功后记得
                        //login_success.store(false);
                        //state=main_menu;

            // 记得用waiting等待输出|||||||||||||||||            
                        continue;
                    }

                    if(type=="quit_account"){
                        thread_pool.enqueue([this]() {
                            quit_account_msg();
                            sem_post(&this->sem);  // 通过 this 访问成员变量
                        });


                            // login_success.store(false);
                            // state=main_menu;
                            // flushInput();
                            // waiting();
                       // 成功失败都要再这里面实现交互逻辑
                        continue;
                    }

                    if(type=="username_view"){
                        thread_pool.enqueue([this, fd, j]() {
                            username_view_msg(fd, j,this->token);
                            
                            sem_post(&this->sem);  // 通过 this 访问成员变量
                        });




                        continue;
                    }

                    if(type=="username_change"){
                        thread_pool.enqueue([this, fd, j]() {
                            username_change_msg(fd, j,this->token);
                            
                            sem_post(&this->sem);  // 通过 this 访问成员变量
                        });



                        continue;
                    }

                    if(type=="password_change"){
                        thread_pool.enqueue([this, fd, j]() {
                            password_change_msg(fd, j,this->token);
                            
                            sem_post(&this->sem);  // 通过 this 访问成员变量
                        });
                            // 要有修改密码成功的提示，用waiting


                        continue;
                    }

                    if(type==""){
                                        


                    // if(type=="log_in"){
                    // thread_pool->enqueue([fd, request]() {
                    // sign_up_msg(fd,request);
                    // });

                    // 服务端发来的json好像不一样要修改逻辑

                    // 这里处理接收到的json消息


                    // 例如：
                    // handle_server_message(j);

                    // 根据协议，处理完毕后可能需要 sem_post() 解锁等待线程

                    // sem_post(&sem);







                    
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

  MenuState state = main_menu; // 初始化界面
    
    // signal(SIGINT, SIG_IGN);   // 忽略 Ctrl+C (中断信号)
    // signal(SIGTERM, SIG_IGN);  // 忽略 kill 发送的终止信号
    // signal(SIGTSTP, SIG_IGN);  // 忽略 Ctrl+Z (挂起/停止信号)

    // struct termios tty;
    // if (tcgetattr(STDIN_FILENO, &tty) == 0) {
    //     tty.c_cc[VEOF] = 0;
    //     tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    // }


    // 交互逻辑，比如说注册函数之类
    // 也就是客户端怎么发送json到服务端
    // 记得通过信号量来等待

    while(running){

        // 登录过期检测
        if (!login_success.load() && state != main_menu) {
            std::cout << "请重新登录。" << std::endl;


            // 不知道哪里清除


            // current_UID.clear();          // 清除登录状态
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
                case 2: 
                case 3: 
                case 4: 
                case 5: 
                case 6: 
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
                case 2: destory_account(sock,state,login_success,token,sem); break;
                case 3: quit_account(sock,state,login_success,token,sem); break;
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
                case 1: username_view(); break;
                case 2: username_change(); break;
                case 3: password_change(); break;
                case 4: state=next1_menu; break;
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





