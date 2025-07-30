#include"menu.hpp"

void show_main_menu(){
        system("clear");
        std::cout << "欢迎进入聊天室！" << std::endl;
        std::cout << "1. 登录" << std::endl;
        std::cout << "2. 注册" << std::endl;
        std::cout << "3. 退出" << std::endl;
        //std::cout << "请输入你的选项：";
}

void show_next_menu(){
        system("clear");
        std::cout << "1. 个人中心" << std::endl;
        std::cout << "2. 好友与群聊管理" << std::endl;
        // std::cout << "3. 文件接收" << std::endl;
        // std::cout << "4. 信息通知" << std::endl; 直接实时通知不用实现
        //std::cout << "请输入你的选项：";
}

// 个人中心
void show_next1_menu(){
        system("clear");
        std::cout << "1. 个人信息" << std::endl;
        std::cout << "2. 注销帐号" << std::endl;
        std::cout << "3. 退出帐号" << std::endl;
        std::cout << "4. 返回" << std::endl;
        //std::cout << "请输入你的选项：";
}

// 个人信息
void show_next11_menu(){
        system("clear");
        std::cout << "1. 用户名查看" << std::endl;
        std::cout << "2. 用户名修改" << std::endl;
        std::cout << "3. 密码修改" << std::endl;
        std::cout << "4. 返回" << std::endl;
        //td::cout << "请输入你的选项：";
}

// 好友与群聊管理
void show_next2_menu(){
        system("clear");
        std::cout << "1. 好友管理" << std::endl;
        std::cout << "2. 群管理" << std::endl;
        std::cout << "5. 返回" << std::endl; 
        //std::cout << "6. 历史记录查询" << std::endl; 直接包含在私聊里面不用实现
        //std::cout << "请输入你的选项：";
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
        //std::cout << "7. 文件发送" << std::endl;

        //std::cout << "请输入你的选项：";
}

// 群聊管理
void show_next22_menu(){
        system("clear");
        std::cout << "1. 群聊列表" << std::endl;
        std::cout << "2. 添加群聊" << std::endl;
        std::cout << "3. 退出群聊" << std::endl; // 要已加入
        std::cout << "4. 群成员列表" << std::endl;
        std::cout << "5. 群聊创建" << std::endl;
        std::cout << "6. 管理员设置" << std::endl; // 群主
        std::cout << "7. 管理员移除" << std::endl; // 群主
        std::cout << "8. 移除成员" << std::endl; // 群主 管理员
        std::cout << "9. 添加成员" << std::endl; // 群主 管理员
        std::cout << "10. 解散群" << std::endl; // 群主
        std::cout << "11. 群聊" << std::endl; // 包含历史记录
        std::cout << "12. 群申请" << std::endl; // 群主 管理员
        std::cout << "13. 返回" << std::endl;
        //std::cout << ". 文件发送" << std::endl; // 放群聊
        //std::cout << "请输入你的选项：";
}

// 文件接收
void show_next3_menu(){
        // system("clear");
        ;
}





// // 信息通知
// void show_next4_menu(){
//         system("clear");
//         std::cout << "1. 好友信息" << std::endl;
//         //std::cout << "2. 群聊信息" << std::endl;
//         std::cout << "3. 返回" << std::endl;
//         //std::cout << "4. " << std::endl;




//         std::cout << "请输入你的选项：";
// }



// 退出登录，私聊，发送和查看好友请求，接收文件，黑名单，查询好友，删除好友，      
// 1.退出登录2.私聊"3.发送好友请求" 4.查看好友请求"
// 5.接受文件"6.黑名单操作"7.查询好友" 8.删除好友" 




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