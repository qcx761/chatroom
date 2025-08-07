#include"client.hpp" // 客户端
#include"menu.hpp"  // 目录
#include"json.hpp"  // json发送
#include"account.hpp"// 阻塞函数
#include"msg.hpp"  // 信息处理函数
using namespace std;

Client::Client(std::string ip, int port)
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket failed");
        exit(1);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if (connect(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect failed");
        exit(1);
    }


    int flags=fcntl(sock, F_GETFL) ;
    if (flags == -1) {
        perror("fcntl F_GETFL failed");
        exit(1);
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL failed");
        exit(1);
    }
    epfd = epoll_create1(0);
        if (epfd == -1) {
        perror("epoll_create1 failed");
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
            perror("epoll_wait failed");
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
                        sign_up_msg(j);
                        sem_post(&this->sem);  // 通过 this 访问成员变量
                        continue;
                    }

                    if(type=="log_in"){
                        bool success_if = log_in_msg(j,this->token);
                        this->login_success.store(success_if);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="destory_account"){
                        destory_account_msg(j);
                        state=main_menu;
                        login_success.store(false);
                        token.clear();
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="quit_account"){
                        quit_account_msg(j);
                        state=main_menu;
                        login_success.store(false);
                        token.clear();
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="username_view"){
                        username_view_msg(j);
                        sem_post(&this->sem);        
                        continue;
                    }

                    if(type=="username_change"){
                        username_change_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="password_change"){
                        password_change_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="show_friend_list"){
                        show_friend_list_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }    
                    
                    if(type=="add_friend"){
                        add_friend_msg(j);
                        sem_post(&this->sem);
                        continue;
                    } 

                    if(type=="remove_friend"){
                        remove_friend_msg(j);
                        sem_post(&this->sem);
                        continue;
                    } 

                    if(type=="mute_friend"){
                        mute_friend_msg(j);
                        sem_post(&this->sem);
                        continue;
                    } 

                    if(type=="unmute_friend"){
                        unmute_friend_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="get_friend_requests"){
                        get_friend_requests_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="handle_friend_request"){
                        handle_friend_request_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="get_friend_info"){
                        get_friend_info_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="get_private_history"){                        
                        get_private_history_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="send_private_message"){
                        send_private_message_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }   

                    if(type=="receive_private_message"){
                        receive_private_message_msg(j);
                        continue;
                    }

                    if(type=="get_unread_private_messages"){        
                        get_unread_private_messages_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="show_group_list"){
                        show_group_list_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="join_group"){
                        join_group_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="quit_group"){
                        quit_group_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="show_group_members"){
                        show_group_members_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="create_group"){
                        create_group_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="set_group_admin"){
                        set_group_admin_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="remove_group_admin"){
                        remove_group_admin_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="remove_group_member"){
                        remove_group_member_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="add_group_member"){
                        add_group_member_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="dismiss_group"){
                        dismiss_group_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="get_group_requests"){
                        get_group_requests_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="handle_group_request"){
                        handle_group_request_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="get_unread_group_messages"){
                        get_unread_group_messages_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }
                    if(type=="receive_group_messages"){
                        receive_group_messages_msg(j);
                        continue;
                    }
                    if(type=="get_group_history"){
                        get_group_history_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="send_group_message"){
                        send_group_message_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }   

                    // 处理在线通知
                    if(type=="receive_message"){
                        receive_message_msg(j);
                        continue;
                    }   

                    // 处理离线通知
                    if(type=="offline_summary"){
                        offline_summary_msg(j);
                        continue;
                    }   

                    if(type=="get_file_list"){
                        get_file_list_msg(j);
                        sem_post(&this->sem);
                        continue;
                    }                 
                    
                    




                    // if(type==""){
                    //     _msg(j);
                    //     sem_post(&this->sem);
                    //     continue;
                    // }                      
                    // if(type==""){
                    //     _msg(j);
                    //     sem_post(&this->sem);
                    //     continue;
                    // }  



                    // if(type=="receive_file"){
                    //     _msg(j);
                    //     sem_post(&this->sem);
                    //     continue;
                    // }  














                    // if(type=="send_group_file"){
                    //     send_group_file_msg(j);
                    //     sem_post(&this->sem);
                    //     continue;
                    // }  













                    if(type==""){
                        //_msg(j);
                        //state=main_menu;
                        //login_success.store(false);
                        //token.clear();
                        sem_post(&this->sem);
                        continue;
                    }







                    if(type==""){
                        //_msg(j);
                        //state=main_menu;
                        //login_success.store(false);
                        //token.clear();
                        sem_post(&this->sem);
                        continue;
                    }
                    if(type==""){
                        //_msg(j);
                        //state=main_menu;
                        //login_success.store(false);
                        //token.clear();
                        sem_post(&this->sem);
                        continue;
                    }
                    if(type==""){
                        //_msg(j);
                        //state=main_menu;
                        //login_success.store(false);
                        //token.clear();
                        sem_post(&this->sem);
                        continue;
                    }
                    if(type==""){
                        //_msg(j);
                        //state=main_menu;
                        //login_success.store(false);
                        //token.clear();
                        sem_post(&this->sem);
                        continue;
                    }
                    if(type==""){
                        //_msg(j);
                        //state=main_menu;
                        //login_success.store(false);
                        //token.clear();
                        sem_post(&this->sem);
                        continue;
                    }
                    if(type==""){
                        //_msg(j);
                        //state=main_menu;
                        //login_success.store(false);
                        //token.clear();
                        sem_post(&this->sem);
                        continue;
                    }

                    if(type=="error"){
                    // 处理错误信息
                    }
                    std::cout << "收到未知消息" << std::endl;
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
            perror("未知文件描述符事件");
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
                show_next_menu();
                std::string input = readline_string("请输入选项: ");
                int m;
                try { m = std::stoi(input); }
                catch (...) {
                    std::cout << "无效的输入，请输入数字。" << std::endl;
                    waiting();
                    continue;
                }
                switch (m)
                {
                case 1: state=next1_menu; break; // 进入个人中心
                case 2: state=next2_menu; break;
                case 3: receive_file(sock,token,sem); break;
                default:
                    cout << "无效数字" << endl;
                    waiting();
                }
                break;
            }

            case next1_menu:
            {
                show_next1_menu();
                std::string input = readline_string("请输入选项: ");
                int m;
                try { m = std::stoi(input); }
                catch (...) {
                    std::cout << "无效的输入，请输入数字。" << std::endl;
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
                    waiting();
                }
                break;
            }

            case next11_menu:
            {
                show_next11_menu();
                std::string input = readline_string("请输入选项: ");
                int m;
                try { m = std::stoi(input); }
                catch (...) {
                    std::cout << "无效的输入，请输入数字。" << std::endl;
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
                    waiting();
                }
                break;
            }

            case next2_menu:
            {
                show_next2_menu();
                std::string input = readline_string("请输入选项: ");
                int m;
                try { m = std::stoi(input); }
                catch (...) {
                    std::cout << "无效的输入，请输入数字。" << std::endl;
                    waiting();
                    continue;
                }
                switch (m)
                {
                case 1: state=next21_menu; break;
                case 2: state=next22_menu; break;
                case 3: state=next_menu; break;
                default:
                    cout << "无效数字" << endl;
                    waiting();
                }
                break;
            }

            case next21_menu:
            {
                show_next21_menu();
                std::string input = readline_string("请输入选项: ");
                int m;
                try { m = std::stoi(input); }
                catch (...) {
                    std::cout << "无效的输入，请输入数字。" << std::endl;
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
                case 7: get_friend_info(sock,token,sem); break;
                case 8: getandhandle_friend_request(sock,token,sem); break;
                case 9: state=next2_menu; break;
                default:
                    cout << "无效数字" << endl;
                    waiting();
                }
                break;
            }

            case next22_menu:
            {
                show_next22_menu();
                std::string input = readline_string("请输入选项: ");
                int m;
                try { m = std::stoi(input); }
                catch (...) {
                    std::cout << "无效的输入，请输入数字。" << std::endl;
                    waiting();
                    continue;
                }
                switch (m)
                {
                case 1: show_group_list(sock,token,sem); break;
                case 2: join_group(sock,token,sem); break;
                case 3: quit_group(sock,token,sem); break;
                case 4: show_group_members(sock,token,sem); break;
                case 5: create_group(sock,token,sem); break;
                case 6: set_group_admin(sock,token,sem); break;
                case 7: remove_group_admin(sock,token,sem); break;
                case 8: remove_group_member(sock,token,sem); break;
                case 9: add_group_member(sock,token,sem); break;
                case 10: dismiss_group(sock,token,sem); break;
                case 11: send_group_message(sock,token,sem); break;
                case 12: getandhandle_group_request(sock,token,sem); break;
                case 13: state=next2_menu; break;
                default:
                    cout << "无效数字" << endl;
                    waiting();
                }
                break;
            }
        } // switch处理
    } // while循环
} // 线程





