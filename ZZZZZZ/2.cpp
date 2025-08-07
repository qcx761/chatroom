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
























#pragma once
#include <atomic>
#include <string>
#include <ctime>

class Client {
public:
    Client(std::string ip, int port);
    void run();  // 启动客户端主循环（含心跳检测）

private:
    int sock;
    int epfd;
    std::atomic<time_t> last_response_time;
    static constexpr int TIMEOUT_SECONDS = 60;

    void handle_server_response();
    void handle_timeout_check();
};












#include "Client.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <cstring>

Client::Client(std::string ip, int port)
{
    last_response_time.store(time(NULL));  // 初始化心跳时间

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket failed");
        exit(1);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if (connect(sock, (sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect failed");
        exit(1);
    }

    // 设置非阻塞
    int flags = fcntl(sock, F_GETFL);
    if (flags == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl failed");
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

void Client::run()
{
    epoll_event events[10];

    while (true) {
        int n = epoll_wait(epfd, events, 10, 1000);  // 等待最多1秒
        if (n < 0) {
            perror("epoll_wait error");
            break;
        }

        bool had_response = false;

        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == sock && (events[i].events & EPOLLIN)) {
                handle_server_response();
                had_response = true;
            }
        }

        if (!had_response) {
            handle_timeout_check();
        }

        // 可选：定期向服务器发送心跳
        const char* ping = "PING";
        send(sock, ping, strlen(ping), 0);
    }
}

void Client::handle_server_response()
{
    char buffer[1024];
    int n = recv(sock, buffer, sizeof(buffer), 0);
    if (n <= 0) {
        std::cout << "[客户端] 服务器断开连接。\n";
        close(sock);
        exit(0);
    }

    buffer[n] = '\0';
    std::cout << "[客户端] 收到消息：" << buffer << std::endl;

    // 更新最后响应时间
    last_response_time.store(time(NULL));
}

void Client::handle_timeout_check()
{
    time_t now = time(NULL);
    time_t last = last_response_time.load();

    if (now - last > TIMEOUT_SECONDS) {
        std::cout << "[客户端] 超过 " << TIMEOUT_SECONDS << " 秒未收到服务器响应，断开连接。\n";
        close(sock);
        exit(0);
    }
}


#pragma once
#include <atomic>
#include <string>
#include <ctime>

class Client {
public:
    Client(std::string ip, int port);
    void run();  // 启动客户端主循环（含心跳检测）

private:
    int sock;
    int epfd;
    std::atomic<time_t> last_response_time;
    static constexpr int TIMEOUT_SECONDS = 60;

    void handle_server_response();
    void handle_timeout_check();
};







void epoll_thread_func(Client *client) {
    client->last_response_time.store(time(NULL)); // 初始化心跳计时

    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        return;
    }
    client->epfd = epfd;

    struct epoll_event event{};
    event.data.fd = client->sock;
    event.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, client->sock, &event);

    struct epoll_event events[1024];

    time_t last_ping_time = time(NULL);

    while (client->running) {
        int n = epoll_wait(epfd, events, 1024, 1000); // 1秒超时，方便检测断线

        if (n == -1) {
            perror("epoll_wait");
            break;
        }

        time_t now = time(NULL);

        // 发送心跳包，每30秒一次
        if (now - last_ping_time > 30) {
            const char* ping_msg = R"({"type":"ping"})";
            send(client->sock, ping_msg, strlen(ping_msg), 0);
            last_ping_time = now;
        }

        if (n == 0) {
            // 超时没有事件，检查心跳超时
            if (now - client->last_response_time.load() > client->TIMEOUT_SECONDS) {
                std::cout << "[客户端] 超过 " << client->TIMEOUT_SECONDS << " 秒未收到服务器响应，断开连接。\n";
                client->running = false;
                client->login_success.store(false);
                client->token.clear();
                client->state = main_menu;

                close(client->sock);
                client->sock = -1;
                close(client->epfd);
                client->epfd = -1;

                sem_post(&client->sem);  // 防止死锁
                return;
            }
            continue;
        }

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == client->sock) {
                if (events[i].events & EPOLLIN) {
                    char buffer[1024];
                    ssize_t count = recv(client->sock, buffer, sizeof(buffer) - 1, 0);
                    if (count <= 0) {
                        std::cout << "[客户端] 服务器断开连接。\n";
                        client->running = false;
                        client->login_success.store(false);
                        client->token.clear();
                        client->state = main_menu;

                        close(client->sock);
                        client->sock = -1;
                        close(client->epfd);
                        client->epfd = -1;

                        sem_post(&client->sem);
                        return;
                    }
                    buffer[count] = '\0';

                    // 这里你可以打印或处理消息
                    // std::cout << "[收到] " << buffer << std::endl;

                    // 收到任何消息都重置心跳计时
                    client->last_response_time.store(time(NULL));
                }
            }
        }
    }

    close(client->sock);
    close(client->epfd);
}




// CREATE TABLE messages (
//     id INT AUTO_INCREMENT PRIMARY KEY,      -- 消息唯一ID，自增主键
//     sender VARCHAR(64),                     -- 发送者账号
//     receiver VARCHAR(64),                   -- 接收者账号
//     content TEXT,                           -- 消息内容，文本类型
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,  -- 消息发送时间，默认当前时间
//     is_online BOOLEAN DEFAULT FALSE,       -- 发送时接收者是否在线，默认为否
//     is_read BOOLEAN DEFAULT FALSE          -- 消息是否已读标志，默认为否
// );




// 群消息：group_messages

// CREATE TABLE group_messages (
//     id INT PRIMARY KEY AUTO_INCREMENT,               -- 消息ID
//     group_id INT NOT NULL,                           -- 所属群ID
//     sender VARCHAR(64) NOT NULL,                     -- 发送者账号
//     content TEXT NOT NULL,                           -- 消息内容
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP    -- 发送时间
// );



MEDIUMTEXT

CREATE TABLE messages (
    id INT AUTO_INCREMENT PRIMARY KEY,      -- 消息唯一ID，自增主键
    sender VARCHAR(64),                     -- 发送者账号
    receiver VARCHAR(64),                   -- 接收者账号
    content MEDIUMTEXT,                           -- 消息内容，文本类型
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,  -- 消息发送时间，默认当前时间
    is_online BOOLEAN DEFAULT FALSE,       -- 发送时接收者是否在线，默认为否
    is_read BOOLEAN DEFAULT FALSE          -- 消息是否已读标志，默认为否
);


群消息：group_messages

CREATE TABLE group_messages (
    id INT PRIMARY KEY AUTO_INCREMENT,               -- 消息ID
    group_id INT NOT NULL,                           -- 所属群ID
    sender VARCHAR(64) NOT NULL,                     -- 发送者账号
    content MEDIUMTEXT NOT NULL,                           -- 消息内容
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP    -- 发送时间
);