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
void is_mute_friend_msg_msg(int fd, const json &request);
void remove_friend_msg(int fd, const json &request);
