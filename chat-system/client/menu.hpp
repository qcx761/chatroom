#pragma once
#include<iostream>
enum MenuState {
    main_menu,    // 主菜单
    next_menu,    // 登录后主界面
    next1_menu,   // 个人中心
    next11_menu,  // 个人信息  
    next2_menu,   // 通讯录
    next21_menu,  // 好友
    next22_menu,  // 群聊
};

void show_main_menu();
void show_next_menu();
void show_next1_menu();
void show_next11_menu();
void show_next2_menu();
void show_next3_menu();
void show_next21_menu();
void show_next22_menu();





