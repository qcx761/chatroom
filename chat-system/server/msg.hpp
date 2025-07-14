#pragma once

#include <nlohmann/json.hpp>
#include <sw/redis++/redis++.h>  // Redis 库头文件

using json = nlohmann::json;
using namespace sw::redis;

void error_msg(int fd,const json &request);
void sign_up_msg(int fd, const json &request);
void log_in_msg(int fd, const json &request);