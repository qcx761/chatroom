#include"menu.hpp"

void show_main_menu(){
        system("clear");
        std::cout << "欢迎进入聊天室！" << std::endl;
        std::cout << "1. 登录" << std::endl;
        std::cout << "2. 注册" << std::endl;
        std::cout << "3. 退出" << std::endl;
}

void show_next_menu(){
        system("clear");
        std::cout << "1. 个人中心" << std::endl;
        std::cout << "2. 好友与群聊管理" << std::endl;
        std::cout << "3. 文件接收" << std::endl;
}

// 个人中心
void show_next1_menu(){
        system("clear");
        std::cout << "1. 个人信息" << std::endl;
        std::cout << "2. 注销帐号" << std::endl;
        std::cout << "3. 退出帐号" << std::endl;
        std::cout << "4. 返回" << std::endl;
}

// 个人信息
void show_next11_menu(){
        system("clear");
        std::cout << "1. 用户名查看" << std::endl;
        std::cout << "2. 用户名修改" << std::endl;
        std::cout << "3. 密码修改" << std::endl;
        std::cout << "4. 返回" << std::endl;
}

// 好友与群聊管理
void show_next2_menu(){
        system("clear");
        std::cout << "1. 好友管理" << std::endl;
        std::cout << "2. 群管理" << std::endl;
        std::cout << "3. 返回" << std::endl; 
}

// 好友管理
void show_next21_menu(){
        system("clear");
        std::cout << "1. 好友列表" << std::endl; // 要实现好友查询，显示在线状态
        std::cout << "2. 添加好友" << std::endl;
        std::cout << "3. 删除好友" << std::endl;
        std::cout << "4. 私聊好友" << std::endl; // 必须是好友 包含历史记录
        std::cout << "5. 解除屏蔽好友" << std::endl;
        std::cout << "6. 屏蔽好友" << std::endl; // 屏蔽信息
        std::cout << "7. 查询好友" << std::endl;
        std::cout << "8. 好友申请" << std::endl;
        std::cout << "9. 返回" << std::endl;
}

// 群聊管理
void show_next22_menu(){
        system("clear");
        std::cout << "1. 群聊列表" << std::endl;
        std::cout << "2. 添加群聊" << std::endl;
        std::cout << "3. 退出群聊" << std::endl; // 要已加入
        std::cout << "4. 群成员列表" << std::endl;
        std::cout << "5. 群聊创建" << std::endl;
        std::cout << "6. 管理员添加" << std::endl; // 群主
        std::cout << "7. 管理员移除" << std::endl; // 群主
        std::cout << "8. 移除成员" << std::endl; // 群主 管理员
        std::cout << "9. 添加成员" << std::endl; // 群主 管理员
        std::cout << "10. 解散群" << std::endl; // 群主
        std::cout << "11. 群聊" << std::endl; // 包含历史记录
        std::cout << "12. 群申请" << std::endl; // 群主 管理员
        std::cout << "13. 返回" << std::endl;
}
