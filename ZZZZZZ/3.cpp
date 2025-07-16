enum MenuState {
        main_menu,    // 主菜单

        next_menu,    // 登录后主界面
        next1_menu

        MENU_ACCOUNT, // 账户管理
        MENU_FRIEND,  // 好友管理
        MENU_GROUP,   // 群组管理
        MENU_EXIT     // 退出



};

void Client::user_thread_func() {

    MenuState state = MENU_MAIN;
    
    signal(SIGINT, SIG_IGN);   // 忽略 Ctrl+C (中断信号)
    signal(SIGTERM, SIG_IGN);  // 忽略 kill 发送的终止信号
    signal(SIGTSTP, SIG_IGN);  // 忽略 Ctrl+Z (挂起/停止信号)

    struct termios tty;
    if (tcgetattr(STDIN_FILENO, &tty) == 0) {
        tty.c_cc[VEOF] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    }


// 交互逻辑，比如说注册函数之类
// 也就是客户端怎么发送json到服务端
// 记得通过信号量来等待

    while(running){

        // 登录过期检测
        if (!login_success.load() && state != main_menu) {
            std::cout << "登录已过期，请重新登录。" << std::endl;



            // 不知道哪里清除

            // current_UID.clear();             // 清除登录状态

            state = main_menu;               // 返回登录页
            waiting();                       // 等待用户确认
            continue;
        }

        switch(state)
        {

            case main_menu:
            {
                main_menu_ui(sock,sem,login_success);
                state=next_menu;
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
                case 1: state=next1_menu; break;
                case 2: 
                case 3:
                case 4: state=
                default :
                    cout << "无效数字" << endl;
                    flushInput(); // 去除数字后面的换行符
                    waiting();
                }
                break;
            }
            case next1_menu:
            {

                break;
            }
            case
            {

                break;
            }

        }



        
    }
}





