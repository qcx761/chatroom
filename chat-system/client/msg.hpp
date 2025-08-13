#pragma once

#include <nlohmann/json.hpp>
// #include <sw/redis++/redis++.h>  // Redis 库头文件
#include <iostream>
#include <vector>
#include <readline/readline.h>
#include <readline/history.h>
#include <mutex>


using json = nlohmann::json;
// using namespace sw::redis;


// 定义
// 获取好友申请列表
extern std::vector<json> global_friend_requests;
extern std::mutex friend_requests_mutex;

// 用来知道非阻塞线程操作的哪个好友
extern std::string current_chat_target;


// 定义
// 获取群申请列表
extern std::vector<json> global_group_requests;
extern std::mutex group_requests_mutex;

// 用来知道非阻塞线程操作的哪个群
extern std::string current_chat_group;

// 定义
// 用户的好友列表
extern std::vector<std::string> friend_list;
extern std::unordered_map<std::string,bool> friend_bemuted_map;



// 定义
// 用户的群聊列表
extern std::vector<std::string> group_list;


// 定义
// 用户的文件列表
extern std::vector<std::string> file_list;



// 错误处理函数
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
void get_friend_info_msg(const json &response);
void get_friend_requests_msg(const json &response);
void handle_friend_request_msg(const json &response);
void get_private_history_msg(const json &response);
void send_private_message_msg(const json &response);
void receive_private_message_msg(const json &response);
void get_unread_private_messages_msg(const json &response);
void show_group_list_msg(const json &response);
void join_group_msg(const json &response);
void quit_group_msg(const json &response);
void show_group_members_msg(const json &response);
void create_group_msg(const json &response);
void set_group_admin_msg(const json &response);
void remove_group_admin_msg(const json &response);
void remove_group_member_msg(const json &response);
void add_group_member_msg(const json &response);
void dismiss_group_msg(const json &response);
void get_group_requests_msg(const json &response);
void handle_group_request_msg(const json &response);
void get_unread_group_messages_msg(const json &response);
void receive_group_messages_msg(const json &response);
void get_group_history_msg(const json &response);
void send_group_message_msg(const json &response);
void get_file_list_msg(const json &response);
void receive_message_msg(const json &response);
void offline_summary_msg(const json &response);
