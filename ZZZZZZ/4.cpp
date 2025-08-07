#include <iostream>
#include <string>
#include <limits>
#include <atomic>
#include <semaphore.h>
#include <termios.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <json.hpp> // 假设你使用 nlohmann/json
#include <mutex>
#include <sstream>

using namespace std;
using json = nlohmann::json;

extern void send_json(int sock, const json& j);
extern void show_main_menu();
extern vector<json> global_friend_requests;
extern mutex friend_requests_mutex;
extern string current_chat_target;

string readline_string(const string& prompt) {
    char* input = readline(prompt.c_str());
    if (!input) return "";
    string result(input);
    if (!result.empty()) add_history(input);
    free(input);
    return result;
}

void waiting() {
    readline_string("按 Enter 键继续...");
}

void flushInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string get_password(const string& prompt) {
    struct termios oldt, newt;
    cout << prompt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    string password;
    getline(cin, password);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    cout << endl;
    return password;
}

void main_menu_ui(int sock, sem_t& sem, atomic<bool>& login_success) {
    while (!login_success.load()) {
        system("clear");
        show_main_menu();
        int n;

        string input = readline_string("请输入你的选项：");
        try {
            n = stoi(input);
        } catch (...) {
            cout << "无效的输入，请输入数字。" << endl;
            waiting();
            continue;
        }

        switch (n) {
            case 1:
                log_in(sock, sem);
                waiting();
                break;
            case 2:
                sign_up(sock, sem);
                waiting();
                break;
            case 3:
                exit(0);
            default:
                cout << "无效数字" << endl;
                waiting();
                break;
        }
    }
}

void log_in(int sock, sem_t& sem) {
    system("clear");
    cout << "登录" << endl;
    json j;
    j["type"] = "log_in";

    string account = readline_string("请输入帐号   :");
    string password = get_password("请输入密码   :");

    j["account"] = account;
    j["password"] = password;
    send_json(sock, j);
    sem_wait(&sem);
}

void sign_up(int sock, sem_t& sem) {
    system("clear");
    cout << "注册" << endl;
    json j;
    j["type"] = "sign_up";

    string username = readline_string("请输入用户名 :");
    string account = readline_string("请输入帐号  :");
    string password_old = get_password("请输入密码   :");
    string password_new = get_password("请再次输入密码:");

    if (password_new == password_old) {
        j["account"] = account;
        j["username"] = username;
        j["password"] = password_old;
        send_json(sock, j);
        sem_wait(&sem);
    } else {
        cout << "两次密码不一样" << endl;
    }
}

void destory_account(int sock, string token, sem_t& sem) {
    system("clear");
    cout << "注销函数 :" << endl;
    string a = readline_string("确定要注销帐号吗(Y/N) :");

    if (a == "Y" || a == "y") {
        string account = readline_string("请输入帐号  :");
        string password = get_password("请输入密码   :");
        json j;
        j["type"] = "destory_account";
        j["token"] = token;
        j["account"] = account;
        j["password"] = password;
        send_json(sock, j);
        sem_wait(&sem);
    } else {
        cout << "已取消注销" << endl;
    }
    waiting();
}

void quit_account(int sock, string token, sem_t& sem) {
    json j;
    j["type"] = "quit_account";
    j["token"] = token;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void username_view(int sock, string token, sem_t& sem) {
    json j;
    j["type"] = "username_view";
    j["token"] = token;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void username_change(int sock, string token, sem_t& sem) {
    system("clear");
    string username = readline_string("请输入想修改的用户名  :");
    json j;
    j["type"] = "username_change";
    j["token"] = token;
    j["username"] = username;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void password_change(int sock, string token, sem_t& sem) {
    system("clear");
    string password_old = get_password("请输入旧密码   :");
    string password_new = get_password("请输入新密码   :");
    string password_new1 = get_password("请再次输入新密码:");

    if (password_new != password_new1) {
        cout << "两次密码不一样" << endl;
        waiting();
        return;
    }
    json j;
    j["type"] = "password_change";
    j["token"] = token;
    j["old_password"] = password_old;
    j["new_password"] = password_new;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void show_friend_list(int sock, string token, sem_t& sem) {
    system("clear");
    json j;
    j["type"] = "show_friend_list";
    j["token"] = token;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void add_friend(int sock, string token, sem_t& sem) {
    system("clear");
    string target_username = readline_string("请输入想添加的用户名  :");
    json j;
    j["type"] = "add_friend";
    j["token"] = token;
    j["target_username"] = target_username;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void remove_friend(int sock, string token, sem_t& sem) {
    system("clear");
    string username = readline_string("请输入想删除的用户名  :");
    json j;
    j["type"] = "remove_friend";
    j["token"] = token;
    j["target_username"] = username;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void mute_friend(int sock, string token, sem_t& sem) {
    system("clear");
    string username = readline_string("请输入想屏蔽的用户名  :");
    json j;
    j["type"] = "mute_friend";
    j["token"] = token;
    j["target_username"] = username;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void unmute_friend(int sock, string token, sem_t& sem) {
    system("clear");
    string username = readline_string("请输入想解除屏蔽的用户名  :");
    json j;
    j["type"] = "unmute_friend";
    j["token"] = token;
    j["target_username"] = username;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void getandhandle_friend_request(int sock, string token, sem_t& sem) {
    system("clear");
    cout << "好友请求列表" << endl;
    json j;
    j["type"] = "get_friend_requests";
    j["token"] = token;
    send_json(sock, j);
    sem_wait(&sem);

    if (global_friend_requests.empty()) {
        waiting();
        return;
    }

    string input = readline_string("输入处理编号(0 退出): ");
    // int choice = stoi(input);
    int choice;
    try {
        choice = std::stoi(input);
    } catch (const std::exception& e) {
        std::cout << "输入错误已取消处理。" << std::endl;
        waiting();
        return;
    }

    if (choice <= 0 || choice > global_friend_requests.size()) {
        cout << "已取消处理。" << endl;
        waiting();
        return;
    }

    string from_username = global_friend_requests[choice - 1]["username"];
    string op = readline_string("你想如何处理 [" + from_username + "] 的请求？(a=接受, r=拒绝): ");

    string action;
    if (op == "a" || op == "A") {
        action = "accept";
    } else if (op == "r" || op == "R") {
        action = "reject";
    } else {
        cout << "无效操作，已取消处理。" << endl;
        waiting();
        return;
    }

    json m;
    m["type"] = "handle_friend_request";
    m["token"] = token;
    m["from_username"] = from_username;
    m["action"] = action;
    send_json(sock, m);
    sem_wait(&sem);
    waiting();
}

void get_friend_info(int sock, const string& token, sem_t& sem) {
    system("clear");
    string username = readline_string("输入查找的好友名 :");
    json j;
    j["type"] = "get_friend_info";
    j["token"] = token;
    j["target_username"] = username;
    send_json(sock, j);
    sem_wait(&sem);
    waiting();
}

void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    current_chat_target = target_username;

    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem);

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录" << endl;

    string message;
    while (true) {
        message = readline_string("> ");
        if (message == "/exit") {
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
        }

        if (message.rfind("/history", 0) == 0) {
            int count = 10;
            istringstream iss(message);
            string cmd;
            iss >> cmd >> count;

            json history;
            history["type"] = "get_private_history";
            history["token"] = token;
            history["target_username"] = target_username;
            history["count"] = count;
            send_json(sock, history);
            sem_wait(&sem);
            continue;
        }

        json msg;
        msg["type"] = "send_private_message";
        msg["token"] = token;
        msg["target_username"] = target_username;
        msg["message"] = message;
        send_json(sock, msg);
        sem_wait(&sem);
    }
}










void send_private_message(int sock, const string& token, sem_t& sem) {
    system("clear");
    cout << "========== 私聊 ==========" << endl;

    string target_username = readline_string("请输入想私聊的用户名 : ");
    if (target_username.empty()) {
        cout << "[错误] 用户名不能为空" << endl;
        return;
    }
    current_chat_target = target_username;

    // 拉取离线消息
    json req;
    req["type"] = "get_unread_private_messages";
    req["token"] = token;
    req["target_username"] = target_username;
    send_json(sock, req);
    sem_wait(&sem);

    cout << "进入与 [" << target_username << "] 的私聊模式" << endl;
    cout << "输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录" << endl;

    while (true) {
        char* line = readline("> ");
        if (!line) { // Ctrl+D 或错误导致 EOF
            cout << "\n[系统] 输入结束，退出私聊模式。" << endl;
            current_chat_target = "";
            break;
        }
        string message(line);
        free(line);

        if (message.empty()) continue;

        if (message == "/exit") {
            current_chat_target = "";
            cout << "[系统] 已退出私聊模式。" << endl;
            break;
        }

        if (message.rfind("/history", 0) == 0) {
            int count = 10;
            std::istringstream iss(message);
            string cmd;
            iss >> cmd >> count;

            json history;
            history["type"] = "get_private_history";
            history["token"] = token;
            history["target_username"] = target_username;
            history["count"] = count;
            send_json(sock, history);
            sem_wait(&sem);
            continue;
        }

        json msg;
        msg["type"] = "send_private_message";
        msg["token"] = token;
        msg["target_username"] = target_username;
        msg["message"] = message;

        send_json(sock, msg);
        sem_wait(&sem);
    }
}
