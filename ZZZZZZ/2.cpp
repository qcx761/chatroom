
std::string current_UID = "";
sem_t semaphore; // 定义信号量

void main_menu_UI(int connecting_sockfd) {
    //屏蔽ctrl+c等
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    //屏蔽ctrl+d
    struct termios tty;

    // 获取当前终端属性
    if (tcgetattr(STDIN_FILENO, &tty) < 0) {
        perror("tcgetattr");
        return;
    }

    // 关闭EOF处理
    tty.c_cc[VEOF] = 0;

    // 应用新的终端属性
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) < 0) {
        perror("tcsetattr");
    }

    //初始化信号量
    sem_init(&semaphore, 0, 0);

    int n;
    while (1) {
        system("clear");
        std::cout << "欢迎进入聊天室！" << std::endl;
        std::cout << "1. 登录" << std::endl;
        std::cout << "2. 注册" << std::endl;
        std::cout << "3. 找回密码" << std::endl;
        std::cout << "4. 退出" << std::endl;
        std::cout << "请输入：";

        // 读取用户输入
        if (!(std::cin >> n)) {
            std::cin.clear(); // 清除错误标志
            std::cout << "无效的输入，请输入一个数字！" << std::endl;
            waiting_for_input();
            continue; // 重新显示菜单
        }

        switch (n) {
            case 1:
                log_in_UI(connecting_sockfd);
                // LogInfo("2.current_UID = {}", current_UID);
                if(current_UID != "") {
                    waiting_for_input();
                    home_UI(connecting_sockfd, current_UID);
                    continue;
                }
                break;
            case 2:
                sign_up_UI(connecting_sockfd);
                break;
            case 3:
                retrieve_password(connecting_sockfd);
                break;
            case 4:
                exit(0);
            default:
                std::cout << "请正确输入选项！" << std::endl;
                break;
        }
        waiting_for_input();
    }
}

void log_in_UI(int connecting_sockfd) {
    system("clear");
    std::cout << "登录" << std::endl;

    json j;
    j["type"] = "log_in";
    std::string username, password;

    std::cout << "请输入你的用户名：";
    std::cin >> username;
    j["username"] = username;

    struct termios oldt, newt;

    // 获取当前终端设置
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // 关闭回显
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::cout << "请输入你的密码：";
    std::cin >> password;
    j["password"] = password;

    std::cout << std::endl;

    // 恢复终端设置
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    send_json(connecting_sockfd, j);
    sem_wait(&semaphore); // 等待信号量
}

void sign_up_UI(int connecting_sockfd) {
    system("clear");
    std::cout << "注册" << std::endl;

    json j;
    j["type"] = "sign_up";
    std::string username, password, security_question, security_answer;

    std::cout << "请输入你的用户名：";
    std::cin >> username;
    j["username"] = username;

    std::cout << "请输入你的密码：";
    std::cin >> password;
    j["password"] = password;

    std::cout << "请输入你的密保问题：";
    std::cin.ignore();
    std::getline(std::cin, security_question);
    j["security_question"] = security_question;

    std::cout << "请输入你的密保答案：";
    std::getline(std::cin, security_answer);
    j["security_answer"] = security_answer;

    send_json(connecting_sockfd, j);
    sem_wait(&semaphore); // 等待信号量
}

void retrieve_password(int connecting_sockfd) {
    system("clear");
    std::cout << "找回密码" << std::endl;

    json j;
    j["type"] = "retrieve_password";
    std::string username;

    std::cout << "请输入你的用户名：";
    std::cin.ignore(); // 清理输入流，以防止之前输入的换行符干扰
    std::cin >> username;
    j["username"] = username;

    send_json(connecting_sockfd, j);
    sem_wait(&semaphore); // 等待信号量

    json j2;
    j2["type"] = "retrieve_password_confirm_answer";
    std::string security_answer;
    
    j2["username"] = username;

    std::cout << "请输入你的密保答案：";
    std::cin >> security_answer;
    j2["security_answer"] = security_answer;

    send_json(connecting_sockfd, j2);
    sem_wait(&semaphore); // 等待信号量
}

void change_usename_UI(int connecting_sockfd, std::string UID) {
    system("clear");
    std::cout << "更改用户名" << std::endl;

    json j;
    j["type"] = "change_usename";
    std::string new_username;

    j["UID"] = UID;

    std::cout << "请输入你的新用户名：";
    std::cin >> new_username;
    j["new_username"] = new_username;

    send_json(connecting_sockfd, j);

    sem_wait(&semaphore); // 等待信号量
    waiting_for_input();
}

void change_password_UI(int connecting_sockfd, std::string UID) {
    system("clear");
    std::cout << "更改密码" << std::endl;

    json j;
    j["type"] = "change_password";
    std::string old_password, new_password;

    j["UID"] = UID;

    std::cout << "请输入你的旧密码：";
    std::cin >> old_password;
    j["old_password"] = old_password;

    std::cout << "请输入你的新密码：";
    std::cin >> new_password;
    j["new_password"] = new_password;

    send_json(connecting_sockfd, j);

    sem_wait(&semaphore); // 等待信号量
    waiting_for_input();
}

void change_security_question_UI(int connecting_sockfd, std::string UID) {
    system("clear");
    std::cout << "更改密保问题" << std::endl;

    json j;
    j["type"] = "change_security_question";
    std::string password, new_security_question, new_security_answer;

    j["UID"] = UID;

    std::cout << "请输入你的密码：";
    std::cin >> password;
    j["password"] = password;

    std::cout << "请输入你的新密保问题：";
    std::cin >> new_security_question;
    j["new_security_question"] = new_security_question;

    // LogInfo("new_security_question = {}", (j["new_security_question"]));
    
    std::cout << "请输入你的新密保答案：";
    std::cin >> new_security_answer;
    j["new_security_answer"] = new_security_answer;

    // LogInfo("new_security_answer = {}", (j["new_security_answer"]));

    send_json(connecting_sockfd, j);

    sem_wait(&semaphore); // 等待信号量
    waiting_for_input();
}

void log_out_UI(int connecting_sockfd, std::string UID) {
    
    system("clear");
    std::cout << "注销帐号" << std::endl;

    json j;
    j["type"] = "log_out";
    std::string password, confirm_password;

    j["UID"] = UID;

    std::cout << "请输入你的密码：";
    std::cin >> password;
    j["password"] = password;

    std::cout << "请确认你的密码：";
    std::cin >> confirm_password;
    j["confirm_password"] = confirm_password;

    if(j["password"] != j["confirm_password"]) {
        std::cout << "两次密码不一致" << std::endl;
        waiting_for_input();
        return;
    }

    send_json(connecting_sockfd, j);

    sem_wait(&semaphore); // 等待信号量
    waiting_for_input();
}#pragma once

#include "SendJson.h"
#include "Home_UI.h"
#include <iostream>
#include <string>
#include <unistd.h> //文件操作
#include <nlohmann/json.hpp> //JSON库
#include <semaphore.h> //信号量库
#include <termios.h> //回显库
#include <csignal>
#include <cstring>

using json = nlohmann::json;

extern sem_t semaphore;//声明信号量

extern std::string current_UID;  // 声明全局变量

void main_menu_UI(int connecting_sockfd);
void log_in_UI(int connecting_sockfd);
void sign_up_UI(int connecting_sockfd);
void retrieve_password(int connecting_sockfd);
void change_usename_UI(int connecting_sockfd, std::string UID);
void change_password_UI(int connecting_sockfd, std::string UID);
void change_security_question_UI(int connecting_sockfd, std::string UID);
void log_out_UI(int connecting_sockfd, std::string UID);































chat-system
├── server/
│   ├── connection.hpp
│   ├── connection.cpp
│   ├── server.hpp
│   ├── server.cpp
│   ├── main.cpp
│   ├── subreactor.cpp
│   └── subreactor.hpp
├── threadpool/
│   ├── threadpool.hpp
│   └── threadpool.cpp