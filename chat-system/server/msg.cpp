#include "json.hpp"
#include "msg.hpp"

using json = nlohmann::json;
using namespace sw::redis;


// CREATE TABLE users (
//     id INT PRIMARY KEY AUTO_INCREMENT,  -- 主键，自动递增的整数ID
//     info JSON                           -- 一个JSON类型的字段，用来存储结构化的JSON数据
// );



// 获取MySQL连接
std::shared_ptr<sql::Connection> get_mysql_connection() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    auto conn = std::shared_ptr<sql::Connection>(driver->connect("tcp://127.0.0.1:3306", "qcx", "qcx761"));
    conn->setSchema("chatroom");  // 你的数据库名
    return conn;
}

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

// 验证token是否有效（读取redis的json），用token获取account
bool verify_token(const std::string& token, std::string& out_account) {
    try {
        Redis redis("tcp://127.0.0.1:6379");
        std::string token_key = "token:" + token;
        auto val = redis.get(token_key);
        if (val) {
            json token_info = json::parse(*val);
            out_account = token_info.value("account", "");
            return true;
        } else {
            return false; // token 不存在或过期
        }
    } catch (const std::exception& e) {
        std::cerr << "Redis error: " << e.what() << std::endl;
        return false;
    }
}

// 注册处理函数
void sign_up_msg(int fd, const json &request) {
    json response;
    std::string account = request.value("account", "");
    std::string username = request.value("username", "");
    std::string password = request.value("password", "");

    if (account.empty() || username.empty() || password.empty()) {
        response["type"] = "sign_up";
        response["status"] = "error";
        response["msg"] = "Account, username or password cannot be empty";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 检查 account 是否存在，使用 MySQL JSON 查询（假设字段名 info）
        // 通过 conn->prepareStatement 预编译了一条 SQL 查询语句，语句里用 ? 作为占位符
        auto check_account_stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT COUNT(*) FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
        check_account_stmt->setString(1, account);
        auto res_account = check_account_stmt->executeQuery();
        //res_account->next();

        if (res_account->next()&&res_account->getInt(1) > 0) {
            response["type"] = "sign_up";
            response["status"] = "fail";
            response["msg"] = "Account already exists";
        } else {
            // 检查 username 是否存在
            auto check_username_stmt = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("SELECT COUNT(*) FROM users WHERE JSON_EXTRACT(info, '$.username') = ?"));
            check_username_stmt->setString(1, username);
            auto res_username = check_username_stmt->executeQuery();
            // res_username->next();

            if (res_username->next()&&res_username->getInt(1) > 0) {
                response["type"] = "sign_up";
                response["status"] = "fail";
                response["msg"] = "Username already exists";
            } else {
                // 构造json存储用户信息
                json user_info;
                user_info["account"] = account;
                user_info["username"] = username;
                user_info["password"] = password;

                auto insert_stmt = std::unique_ptr<sql::PreparedStatement>(
                    conn->prepareStatement("INSERT INTO users (info) VALUES (?)"));
                insert_stmt->setString(1, user_info.dump());  // 存JSON字符串
                insert_stmt->executeUpdate();

                response["type"] = "sign_up";
                response["status"] = "success";
                response["msg"] = "Registered successfully";
            }
        }
    } catch (const std::exception &e) {
        response["type"] = "sign_up";
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }


    // 直接send_json??
    int n;
    do {
        n = send_json(fd, response);
    } while (n != 0);
}

// 登录处理函数
void log_in_msg(int fd, const json &request) {
    json response;
    std::string account = request.value("account", "");
    std::string password = request.value("password", "");

    if (account.empty() || password.empty()) {
        response["type"] = "log_in";
        response["status"] = "error";
        response["msg"] = "Account or password cannot be empty";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // JSON_EXTRACT(json_doc, path)，用于从 JSON 字段中提取你想要的某个值
        auto stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT info FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
        stmt->setString(1, account);
        auto res = stmt->executeQuery();

        if (!res->next()) {
            response["type"] = "log_in";
            response["status"] = "fail";
            response["msg"] = "Account not found";
            send_json(fd, response);
            return;
        }

        // 从 info 字段获取完整用户 JSON
        std::string info_str = res->getString("info");
        json user_info = json::parse(info_str);

        std::string stored_pass = user_info.value("password", "");
        if (stored_pass != password) {
            response["type"] = "log_in";
            response["status"] = "fail";
            response["msg"] = "Incorrect password";
            send_json(fd, response);
            return;
        }

        // 密码正确，生成token，存redis，存JSON字符串
        Redis redis("tcp://127.0.0.1:6379");
        std::string token = generate_token();
        std::string token_key = "token:" + token;

        
        json token_info = {
            {"account", user_info["account"]},
            {"username", user_info["username"]}
        };

        redis.set(token_key, token_info.dump());
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

    int n;
    do {
        n = send_json(fd, response);
    } while (n != 0);
}

void destory_account_msg(int fd, const json &request){
    json response;
    std::string token = request.value("token", "");
    std::string account = request.value("account", "");
    std::string password = request.value("password", "");

    std::string redis_account;
    if (!verify_token(token, redis_account) || redis_account != account) {
        response["type"] = "destory_account";
        response["status"] = "error";
        response["msg"] = "Invalid or expired token";
        send_json(fd, response);
        return;
    }

    try{
        auto conn = get_mysql_connection();
        auto stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT id, info FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
        stmt->setString(1, account); // 绑定以第一个问号
        auto res = stmt->executeQuery();
        
        if(!res->next()){
            response["type"] = "destory_account";
            response["status"] = "fail";
            response["msg"] = "Account not found";
        }else{
            std::string info_str = res->getString("info");// 获取列名为info
            json user_info = json::parse(info_str);
            if(user_info.value("password", "") != password){
                response["type"] = "destory_account";
                response["status"] = "fail";
                response["msg"] = "Incorrect password";
            }else{
                int id = res->getInt("id");
                auto del_stmt = std::unique_ptr<sql::PreparedStatement>(
                    conn->prepareStatement("DELETE FROM users WHERE id = ?"));
                del_stmt->setInt(1, id); // 返回查询到的行
                del_stmt->executeUpdate(); // 删除信息

                Redis redis("tcp://127.0.0.1:6379");
                redis.del("token:" + token); // 删除 Redis key

                response["type"] = "destory_account";
                response["status"] = "success";
                response["msg"] = "Account deleted";
            }
        }
    } catch (const std::exception &e) {
        response["type"] = "destory_account";
        response["status"] = "error";
        response["msg"] = e.what();
    }
    send_json(fd, response);
}

void quit_account_msg(int fd, const json &request){
    json response;
    std::string token = request.value("token", "");

    try {
        Redis redis("tcp://127.0.0.1:6379");
        redis.del("token:" + token);

        response["type"] = "quit_account";
        response["status"] = "success";
        response["msg"] = "Logged out";
    } catch (const std::exception &e) {
        response["type"] = "quit_account";
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);

}

void username_view_msg(int fd, const json &request){
    json response;
    std::string token = request.value("token", "");
    std::string account;

    if (!verify_token(token, account)) {
        response["type"] = "username_view";
        response["status"] = "error";
        response["msg"] = "Invalid or expired token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();
        auto stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT info FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
        stmt->setString(1, account);
        auto res = stmt->executeQuery();

        if (res->next()) {
            json info = json::parse(res->getString("info").asStdString());

            // json info = json::parse(res->getString("info"));
            response["type"] = "username_view";
            response["status"] = "success";
            response["username"] = info["username"];
            response["msg"] = "view success";
        } else {
            response["type"] = "username_view";
            response["status"] = "fail";
            response["msg"] = "User not found";
        }
    } catch (const std::exception &e) {
        response["type"] = "username_view";
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}




void username_change_msg(int fd, const json &request){
    json response;
    std::string token = request.value("token", "");
    std::string new_username = request.value("username", "");
    std::string account;

    if (!verify_token(token, account)) {
        response["type"] = "username_change";
        response["status"] = "error";
        response["msg"] = "Invalid or expired token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 检查用户名是否已存在
        auto check_stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT COUNT(*) FROM users WHERE JSON_EXTRACT(info, '$.username') = ?"));
        check_stmt->setString(1, new_username);
        auto res_check = check_stmt->executeQuery();
        res_check->next();
        if (res_check->getInt(1) > 0) {
            response["type"] = "username_change";
            response["status"] = "fail";
            response["msg"] = "Username already taken";
            send_json(fd, response);
            return;
        }

        // 查询当前 info
        auto select_stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT id, info FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
        select_stmt->setString(1, account);
        auto res = select_stmt->executeQuery();
        if (res->next()) {
            int id = res->getInt("id");
            json info = json::parse(std::string(res->getString("info")));

            // json info = json::parse(res->getString("info"));
            info["username"] = new_username;

            auto update_stmt = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("UPDATE users SET info = ? WHERE id = ?"));
            update_stmt->setString(1, info.dump());
            update_stmt->setInt(2, id);
            update_stmt->executeUpdate();

            response["type"] = "username_change";
            response["status"] = "success";
            response["msg"] = "Username updated";
        } else {
            response["type"] = "username_change";
            response["status"] = "fail";
            response["msg"] = "Account not found";
        }
    } catch (const std::exception &e) {
        response["type"] = "username_change";
        response["status"] = "error";
        response["msg"] = e.what();
    }

    // 修改redis
    try {
        auto redis = sw::redis::Redis("tcp://127.0.0.1:6379");
        std::string token_key = "token:" + token;

        std::string token_json_str = redis.get(token_key).value_or("");
        if (!token_json_str.empty()) {
            json token_info = json::parse(token_json_str);
            token_info["username"] = new_username;
            redis.set(token_key, token_info.dump());
        }
    } catch (const std::exception &e) {
        std::cerr << "Redis update error: " << e.what() << std::endl;
    }

    send_json(fd, response);
}


void password_change_msg(int fd, const json &request){
    
        json response;
    std::string token = request.value("token", "");
    std::string old_pass = request.value("old_password", "");
    std::string new_pass = request.value("new_password", "");
    std::string account;

    if (!verify_token(token, account)) {
        response["type"] = "password_change";
        response["status"] = "error";
        response["msg"] = "Invalid or expired token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();
        auto select_stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT id, info FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
        select_stmt->setString(1, account);
        auto res = select_stmt->executeQuery();

        if (res->next()) {
            int id = res->getInt("id");
            json info = json::parse(std::string(res->getString("info")));

            // json info = json::parse(res->getString("info"));
            if (info["password"] != old_pass) {
                response["type"] = "password_change";
                response["status"] = "fail";
                response["msg"] = "Old password incorrect";
            } else {
                info["password"] = new_pass;
                auto update_stmt = std::unique_ptr<sql::PreparedStatement>(
                    conn->prepareStatement("UPDATE users SET info = ? WHERE id = ?"));
                update_stmt->setString(1, info.dump());
                update_stmt->setInt(2, id);
                update_stmt->executeUpdate();

                response["type"] = "password_change";
                response["status"] = "success";
                response["msg"] = "Password changed";
            }
        } else {
            response["type"] = "password_change";
            response["status"] = "fail";
            response["msg"] = "Account not found";
        }
    } catch (const std::exception &e) {
        response["type"] = "password_change";
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}





// void _msg(int fd, const json &request){



// }


















// resp["type"] = type;
// resp["status"] = "error";
// resp["msg"] = "Invalid or expired token.";






















































void error_msg(int fd, const nlohmann::json &request){
    json response;
    response["type"] = "error";
    response["msg"] = "Unrecognized request type";
    send_json(fd, response);
}



// // 登出函数
// void log_out_msg(const std::string& token) {
//     try {
//         Redis redis("tcp://127.0.0.1:6379");
//         std::string token_key = "token:" + token;
//         redis.del(token_key);
//     } catch (const std::exception& e) {
//         std::cerr << "Logout error: " << e.what() << std::endl;
//     }
// }


