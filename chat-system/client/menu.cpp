#include"menu.hpp"

void show_main_menu(){
        system("clear");
        std::cout << "欢迎进入聊天室！" << std::endl;
        std::cout << "1. 登录" << std::endl;
        std::cout << "2. 注册" << std::endl;
        std::cout << "3. 退出" << std::endl;
        // std::cout << "请输入：";
}

void show_next_menu(){
        system("clear");
        std::cout << "1. 消息" << std::endl;
        std::cout << "2. 通讯录" << std::endl;
        std::cout << "3. 接收文件" << std::endl;
        std::cout << "4. 个人中心" << std::endl;
        // std::cout << "请输入：";
}