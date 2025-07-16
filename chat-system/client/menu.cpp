#include"menu.hpp"

void show_main_menu(){
        system("clear");
        std::cout << "欢迎进入聊天室！" << std::endl;
        std::cout << "1. 登录" << std::endl;
        std::cout << "2. 注册" << std::endl;
        std::cout << "3. 退出" << std::endl;
        std::cout << "请输入你的选项：";
}


void show_next_menu(){
        system("clear");
        std::cout << "1. 个人中心" << std::endl;
        std::cout << "2. 消息" << std::endl;
        std::cout << "3. 接收文件" << std::endl;
        std::cout << "4. 通讯录" << std::endl;
        std::cout << "5. " << std::endl;
        std::cout << "6. " << std::endl;
        std::cout << "7. " << std::endl;
        std::cout << "8. " << std::endl;

        std::cout << "请输入你的选项：";
}

// 个人中心
void show_next1_menu(){
        system("clear");
        std::cout << "1. 个人信息" << std::endl;
        std::cout << "2. 注销帐号" << std::endl;
        std::cout << "3. 退出帐号" << std::endl;
        std::cout << "4. 返回" << std::endl;
        std::cout << "请输入你的选项：";
}

// 个人信息
void show_next11_menu(){
        system("clear");
        std::cout << "1. 用户名查看" << std::endl;
        std::cout << "2. 用户名修改" << std::endl;
        std::cout << "3. 密码修改" << std::endl;
        std::cout << "4. 返回" << std::endl;
        std::cout << "请输入你的选项：";
}

// 多级菜单是如何实现的，要可以返回上级菜单
// 或者比如最内层菜单是退出登录，我要可以直接返回初始菜单，要怎么实现
// 退出登录，私聊，发送和查看好友请求，接收文件，黑名单，查询好友，删除好友，      



// 账号管理

// 实现登录、注册、注销
// 实现通过验证码登录/注册/密码找回（邮件/手机号等）（提高）
// 实现数据加密（提高）
// 好友管理
// 实现好友的添加、删除、查询操作
// 实现显示好友在线状态
// 禁止不存在好友关系的用户间的私聊
// 实现屏蔽好友消息
// 实现好友间聊天







// 群管理

// 实现群组的创建、解散
// 实现用户申请加入群组
// 实现用户查看已加入的群组
// 实现群组成员退出已加入的群组
// 实现群组成员查看群组成员列表
// 实现群主对群组管理员的添加和删除
// 实现群组管理员批准用户加入群组
// 实现群组管理员/群主从群组中移除用户
// 实现群组内聊天功能
// 聊天功能
// 对于 私聊和群组 的聊天功能均需要实现：






// 实现查看历史消息记录

// 实现用户间在线聊天
// 实现在线用户对离线用户发送消息，离线用户上线后获得通知
// 实现文件发送的断点续传（提高）
// 实现在线发送文件
// 实现在线用户对离线用户发送文件，离线用户上线后获得通知/接收
// 实现用户在线时,消息的实时通知
// 收到好友请求
// 收到私聊
// 收到加群申请
// …