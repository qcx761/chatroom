#include <iostream>
#include <stack>
#include <string>
#include <limits>

// 菜单状态枚举
enum MenuState {
    MENU_MAIN,          // 主菜单
    MENU_NEXT,          // 二级菜单
    MENU_USER_CENTER,   // 个人中心
    MENU_USER_INFO,     // 用户信息
    MENU_EXIT           // 退出
};

// 当前用户状态
struct UserState {
    bool is_logged_in = false;
    std::string username;
};

// 清除输入缓冲区
void flushInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// 等待用户按键
void pressAnyKeyToContinue() {
    std::cout << "\n按任意键继续...";
    flushInput();
    std::cin.get();
}

// 菜单显示函数
void show_menu(MenuState state) {
    system("clear"); // Linux/macOS 清屏
    // system("cls"); // Windows 用这个

    switch (state) {
        case MENU_MAIN:
            std::cout << "=== 主菜单 ===\n"
                      << "1. 登录\n"
                      << "2. 注册\n"
                      << "3. 退出\n"
                      << "请选择: ";
            break;

        case MENU_NEXT:
            std::cout << "=== 功能菜单 ===\n"
                      << "1. 个人中心\n"
                      << "2. 消息\n"
                      << "3. 返回上级\n"
                      << "4. 退出\n"
                      << "请选择: ";
            break;

        case MENU_USER_CENTER:
            std::cout << "=== 个人中心 ===\n"
                      << "1. 个人信息\n"
                      << "2. 修改密码\n"
                      << "3. 返回上级\n"
                      << "4. 退出\n"
                      << "请选择: ";
            break;

        case MENU_USER_INFO:
            std::cout << "=== 个人信息 ===\n"
                      << "1. 查看资料\n"
                      << "2. 修改头像\n"
                      << "3. 返回上级\n"
                      << "4. 退出\n"
                      << "请选择: ";
            break;

        default:
            break;
    }
}

// 模拟登录函数
bool mock_login() {
    std::string username, password;
    std::cout << "用户名: ";
    std::cin >> username;
    std::cout << "密码: ";
    std::cin >> password;

    // 简单模拟验证
    return !username.empty() && !password.empty();
}

// 主控制函数
void menu_system() {
    std::stack<MenuState> menu_stack;
    menu_stack.push(MENU_MAIN); // 初始化主菜单
    UserState user;

    while (!menu_stack.empty()) {
        MenuState current = menu_stack.top();
        show_menu(current);

        int choice;
        if (!(std::cin >> choice)) {
            flushInput();
            std::cout << "输入无效，请重新输入！\n";
            pressAnyKeyToContinue();
            continue;
        }

        switch (current) {
            case MENU_MAIN:
                if (choice == 1) {
                    if (mock_login()) {
                        user.is_logged_in = true;
                        menu_stack.push(MENU_NEXT);
                    } else {
                        std::cout << "登录失败！\n";
                        pressAnyKeyToContinue();
                    }
                } else if (choice == 3) {
                    menu_stack.push(MENU_EXIT);
                }
                break;

            case MENU_NEXT:
                if (choice == 1) {
                    menu_stack.push(MENU_USER_CENTER);
                } else if (choice == 3) {
                    menu_stack.pop(); // 返回上级
                } else if (choice == 4) {
                    menu_stack.push(MENU_EXIT);
                }
                break;

            case MENU_USER_CENTER:
                if (choice == 1) {
                    menu_stack.push(MENU_USER_INFO);
                } else if (choice == 3) {
                    menu_stack.pop(); // 返回上级
                } else if (choice == 4) {
                    menu_stack.push(MENU_EXIT);
                }
                break;

            case MENU_USER_INFO:
                if (choice == 3) {
                    menu_stack.pop(); // 返回上级
                } else if (choice == 4) {
                    menu_stack.push(MENU_EXIT);
                }
                break;

            case MENU_EXIT:
                if (choice == 1) {
                    while (!menu_stack.empty()) {
                        menu_stack.pop(); // 清空栈退出
                    }
                } else if (choice == 2) {
                    menu_stack.pop(); // 取消退出
                }
                break;
        }
    }
    std::cout << "系统已退出\n";
}

int main() {
    menu_system();
    return 0;
}