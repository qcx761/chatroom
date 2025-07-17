#pragma once

#include <nlohmann/json.hpp>
#include <sw/redis++/redis++.h>  // Redis 库头文件
#include<iostream>

using json = nlohmann::json;
using namespace sw::redis;




// 错误处理函数要怎么实现
void error_msg(int fd,const json &request);





void sign_up_msg(const json &request);
bool log_in_msg(const json &request,std::string& token);



void destory_account_msg();
void quit_account_msg();
void username_view_msg();
void username_change_msg();
void password_change_msg();