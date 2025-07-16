#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <csignal>
#include <semaphore.h>
#include "Account_UI.h"
#include "../log/mars_logger.h"

extern std::string current_UID;
extern sem_t semaphore;

// 菜单状态枚举
enum MenuState {
    MENU_MAIN,
    MENU_HOME,
    MENU_ACCOUNT,
    MENU_FRIEND,
    MENU_GROUP,
    MENU_EXIT
};

// 等待用户按任意键继续
void waiting_for_input() {
    std::cout << "按任意键继续...";
    std::cin.ignore();
    std::cin.get();
}

// 主菜单处理函数
void main_menu_UI(int connecting_sockfd) {
    MenuState state = MENU_MAIN;
    bool running = true;

    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    struct termios tty;
    if (tcgetattr(STDIN_FILENO, &tty) == 0) {
        tty.c_cc[VEOF] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    }

    sem_init(&semaphore, 0, 0);

    while (running) {
        switch (state) {
            case MENU_MAIN: {
                int n;
                system("clear");
                std::cout << "欢迎进入聊天室！" << std::endl;
                std::cout << "1. 登录" << std::endl;
                std::cout << "2. 注册" << std::endl;
                std::cout << "3. 找回密码" << std::endl;
                std::cout << "4. 退出" << std::endl;
                std::cout << "请输入：";
                if (!(std::cin >> n)) {
                    std::cin.clear();
                    std::cin.ignore(1024, '\n');
                    std::cout << "无效输入，请重试。\n";
                    waiting_for_input();
                    continue;
                }

                switch (n) {
                    case 1:
                        log_in_UI(connecting_sockfd);
                        if (!current_UID.empty()) state = MENU_HOME;
                        break;
                    case 2:
                        sign_up_UI(connecting_sockfd);
                        break;
                    case 3:
                        retrieve_password(connecting_sockfd);
                        break;
                    case 4:
                        running = false;
                        break;
                    default:
                        std::cout << "请正确输入选项！" << std::endl;
                        waiting_for_input();
                }
                break;
            }

            case MENU_HOME: {
                int n;
                system("clear");
                std::cout << "== 主菜单 ==" << std::endl;
                std::cout << "1. 账号管理" << std::endl;
                std::cout << "2. 好友管理" << std::endl;
                std::cout << "3. 群组管理" << std::endl;
                std::cout << "4. 注销登录" << std::endl;
                std::cout << "5. 返回登录页" << std::endl;
                std::cout << "请输入：";
                std::cin >> n;

                switch (n) {
                    case 1: state = MENU_ACCOUNT; break;
                    case 2: state = MENU_FRIEND; break;
                    case 3: state = MENU_GROUP; break;
                    case 4:
                        log_out_UI(connecting_sockfd, current_UID);
                        current_UID.clear();
                        state = MENU_MAIN;
                        break;
                    case 5:
                        current_UID.clear();
                        state = MENU_MAIN;
                        break;
                    default:
                        std::cout << "无效输入！\n";
                        waiting_for_input();
                }
                break;
            }

            case MENU_ACCOUNT: {
                int n;
                system("clear");
                std::cout << "== 账号管理 ==" << std::endl;
                std::cout << "1. 修改用户名" << std::endl;
                std::cout << "2. 修改密码" << std::endl;
                std::cout << "3. 修改密保" << std::endl;
                std::cout << "4. 返回上一级" << std::endl;
                std::cout << "5. 返回主菜单" << std::endl;
                std::cout << "请输入：";
                std::cin >> n;

                switch (n) {
                    case 1: change_usename_UI(connecting_sockfd, current_UID); break;
                    case 2: change_password_UI(connecting_sockfd, current_UID); break;
                    case 3: change_security_question_UI(connecting_sockfd, current_UID); break;
                    case 4: state = MENU_HOME; break;
                    case 5: state = MENU_HOME; break;
                    default:
                        std::cout << "无效输入。\n";
                        waiting_for_input();
                }
                break;
            }

            case MENU_FRIEND: {
                system("clear");
                std::cout << "== 好友管理 [待实现] ==" << std::endl;
                std::cout << "按任意键返回主菜单..." << std::endl;
                waiting_for_input();
                state = MENU_HOME;
                break;
            }

            case MENU_GROUP: {
                system("clear");
                std::cout << "== 群组管理 [待实现] ==" << std::endl;
                std::cout << "按任意键返回主菜单..." << std::endl;
                waiting_for_input();
                state = MENU_HOME;
                break;
            }

            case MENU_EXIT: {
                running = false;
                break;
            }
        }
    }

    std::cout << "程序退出。\n";
}
