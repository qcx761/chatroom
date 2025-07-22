#pragma once

#include <nlohmann/json.hpp>
#include <sw/redis++/redis++.h>  // Redis 库头文件
#include<iostream>

using json = nlohmann::json;
using namespace sw::redis;

// std::vector<json> global_friend_requests;
// std::mutex friend_requests_mutex;


extern std::vector<json> global_friend_requests;
extern std::mutex friend_requests_mutex;




// 错误处理函数要怎么实现
void error_msg(int fd,const json &request);





void sign_up_msg(const json &request);
bool log_in_msg(const json &request,std::string& token);

void destory_account_msg(const json &response);
void quit_account_msg(const json &response);
void username_view_msg(const json &response);
void username_change_msg(const json &response);
void password_change_msg(const json &response);







void show_friend_list_msg(const json &response);
void add_friend_msg(const json &response);
void remove_friend_msg(const json &response);
void mute_friend_msg(const json &response);
void unmute_friend_msg(const json &response);




void get_friend_request_msg(const json &response);
void handle_friend_request_msg(const json &response);
void show_friend_notifications_msg(const json &response);