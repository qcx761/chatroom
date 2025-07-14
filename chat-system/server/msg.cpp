#include "json.hpp"
#include "msg.hpp"

#include <sw/redis++/redis++.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <random>
#include <sstream>
#include <memory>
#include <iostream>

using json = nlohmann::json;
using namespace sw::redis;

// 生成32位随机Token字符串
std::string generate_token() {
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static std::default_random_engine engine(std::random_device{}());
    static std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);

    std::stringstream ss;   
    for (int i = 0; i < 32; ++i) {
        ss << charset[dist(engine)];
    }
    return ss.str();
}

// 获取MySQL连接
std::shared_ptr<sql::Connection> get_mysql_connection() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    auto conn = std::shared_ptr<sql::Connection>(driver->connect("tcp://127.0.0.1:3306", "qcx", "qcx761"));
    conn->setSchema("chatroom");
    return conn;
}

// 注册处理函数
void sign_up_msg(int fd, const json &request) {
    json response;
    std::string username = request.value("username", "");
    std::string password = request.value("password", "");

    if (username.empty() || password.empty()) {
        response["type"] = "sign_up";
        response["status"] = "error";
        response["msg"] = "Username or password cannot be empty";
        send_json(fd, response);
        return;
    }

    try {
        // 获得一个指向数据库连接的智能指针 conn
        auto conn = get_mysql_connection();

        auto check_stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT COUNT(*) FROM users WHERE username = ?"));
        check_stmt->setString(1, username);
        auto res = check_stmt->executeQuery();
        res->next();

        if (res->getInt(1) > 0) {
            response["type"] = "sign_up";
            response["status"] = "fail";
            response["msg"] = "Username already exists";
        } else {
            auto insert_stmt = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("INSERT INTO users (username, password) VALUES (?, ?)"));
            insert_stmt->setString(1, username);
            insert_stmt->setString(2, password);  // 建议密码使用哈希后存储，这里简化示例
            insert_stmt->executeUpdate();

            response["type"] = "sign_up";
            response["status"] = "success";
            response["msg"] = "Registered successfully";
        }
    } catch (const std::exception &e) {
        response["type"] = "sign_up";
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 登录处理函数
void log_in_msg(int fd, const json &request) {
    json response;
    std::string username = request.value("username", "");
    std::string password = request.value("password", "");

    if (username.empty() || password.empty()) {
        response["type"] = "log_in";
        response["status"] = "error";
        response["msg"] = "Username or password cannot be empty";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        auto stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT password FROM users WHERE username = ?"));
        stmt->setString(1, username);
        auto res = stmt->executeQuery();

        if (!res->next()) {
            response["type"] = "log_in";
            response["status"] = "fail";
            response["msg"] = "User not found";
            send_json(fd, response);
            return;
        }

        std::string stored_pass = res->getString("password");
        if (stored_pass != password) {
            response["type"] = "log_in";
            response["status"] = "fail";
            response["msg"] = "Incorrect password";
            send_json(fd, response);
            return;
        }

        // 密码正确，生成token，存redis
        Redis redis("tcp://127.0.0.1:6379");
        std::string token = generate_token();
        std::string token_key = "token:" + token;
        redis.set(token_key, username);
        redis.expire(token_key, 3600);  // 1小时有效期

        response["type"] = "log_in";
        response["status"] = "success";
        response["msg"] = "Login successful";
        response["token"] = token;
    } catch (const std::exception &e) {
        response["type"] = "log_in";
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 登出函数
void log_out_msg(const std::string& token) {
    try {
        Redis redis("tcp://127.0.0.1:6379");
        std::string token_key = "token:" + token;
        redis.del(token_key);
    } catch (const std::exception& e) {
        std::cerr << "Logout error: " << e.what() << std::endl;
    }
}






// 验证token是否有效
bool verify_token(const std::string& token, std::string& out_username) {
    try {
        Redis redis("tcp://127.0.0.1:6379");
        std::string token_key = "token:" + token;
        auto val = redis.get(token_key);
        if (val) {
            out_username = *val;
            return true;
        } else {
            return false; // token 不存在或过期
        }
    } catch (const std::exception& e) {
        std::cerr << "Redis error: " << e.what() << std::endl;
        return false;
    }
}
