#include <iostream>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>

// 假设有这些辅助函数声明
std::string readline_string(const std::string& prompt);
void flushInput();
void waiting();

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
            case main_menu:
            {
                main_menu_ui(sock,sem,login_success);
                state=next_menu; // 进入主界面
                break;
            }

            case next_menu:
            {
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
                case 1: state=next1_menu; break;
                case 2: state=next2_menu; break;
                case 3:
                default:
                    std::cout << "无效数字" << std::endl;
                    waiting();
                }
                break;
            }

            case next1_menu:
            {
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
                case 2: destory_account(sock,token,sem); break;
                case 3: quit_account(sock,token,sem); break;
                case 4: state=next_menu; break;
                default:
                    std::cout << "无效数字" << std::endl;
                    waiting();
                }
                break;
            }

            case next11_menu:
            {
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
                    std::cout << "无效数字" << std::endl;
                    waiting();
                }
                break;
            }

            case next2_menu:
            {
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
                case 3: getandhandle_friend_request(sock,token,sem); break;
                case 4:
                case 5:
                case 6: state=next_menu; break;
                case 7:
                case 8:
                default:
                    std::cout << "无效数字" << std::endl;
                    waiting();
                }
                break;
            }

            case next21_menu:
            {
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
                case 7:
                case 8: get_friend_info(sock,token,sem); break;
                case 9: state=next2_menu; break;
                default:
                    std::cout << "无效数字" << std::endl;
                    waiting();
                }
                break;
            }

            case next22_menu:
            {
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
                    std::cout << "无效数字" << std::endl;
                    waiting();
                }
                break;
            }
        }
    }
}

std::string readline_string(const std::string& prompt) {
    char* input = readline(prompt.c_str());
    if (!input) return "";
    std::string result(input);
    if (!result.empty()) {
        add_history(input);
    }
    free(input);
    return result;
}
