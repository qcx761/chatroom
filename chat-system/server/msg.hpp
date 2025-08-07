#pragma once

#include <nlohmann/json.hpp>
#include <sw/redis++/redis++.h>  // Redis 库头文件

#include <sw/redis++/redis++.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

#include <random>
#include <string.h>
#include <sstream>
#include <memory>
#include <iostream>

using json = nlohmann::json;
using namespace sw::redis;

extern sw::redis::Redis redis;

void error_msg(int fd, const json &request);
std::shared_ptr<sql::Connection> get_mysql_connection();
std::string generate_token();
bool verify_token(const std::string& token, std::string& out_account);
void sign_up_msg(int fd, const json &request);
void log_in_msg(int fd, const json &request);

void destory_account_msg(int fd, const json &request);
void quit_account_msg(int fd, const json &request);
void username_view_msg(int fd, const json &request);
void username_change_msg(int fd, const json &request);
void password_change_msg(int fd, const json &request);


void show_friend_list_msg(int fd, const json &request);
void add_friend_msg(int fd, const json &request);
void mute_friend_msg(int fd, const json &request);
void unmute_friend_msg(int fd, const json &request);
void remove_friend_msg(int fd, const json &request);
void add_friend_msg(int fd, const json &request);


void handle_friend_request_msg(int fd, const json &request);
void get_friend_requests_msg(int fd, const json& request);


void get_friend_info_msg(int fd, const json& request);


void send_private_message_msg(int fd, const json& request);
void get_private_history_msg(int fd, const json& request);
void get_unread_private_messages_msg(int fd, const json& request);





void show_group_list_msg(int fd, const json& request);
void join_group_msg(int fd, const json& request);
void quit_group_msg(int fd, const json& request);
void show_group_members_msg(int fd, const json& request);
void create_group_msg(int fd, const json& request);
void set_group_admin_msg(int fd, const json& request);
void remove_group_admin_msg(int fd, const json& request);
void remove_group_member_msg(int fd, const json& request);
void add_group_member_msg(int fd, const json& request);
void dismiss_group_msg(int fd, const json& request);
void get_unread_group_messages_msg(int fd, const json& request);
void get_group_history_msg(int fd, const json& request);
void send_group_message_msg(int fd, const json& request);
void get_group_requests_msg(int fd, const json& request);
void handle_group_request_msg(int fd, const json& request);






void send_group_file_msg(int fd, const json& request);
void send_private_file_msg(int fd, const json& request);
void get_file_list_msg(int fd, const json& request);



void send_offline_summary_on_login(const std::string& account, int fd);

