#include "json.hpp"
#include "msg.hpp"

using json = nlohmann::json;
using namespace sw::redis;

// 用户表：users
// CREATE TABLE users (
//     id INT PRIMARY KEY AUTO_INCREMENT,  -- 主键，自动递增的整数ID
//     info JSON                           -- 一个JSON类型的字段，用来存储结构化的JSON数据
// );

// CREATE TABLE friends (
//     account VARCHAR(64) PRIMARY KEY,     -- 当前用户账号，主键
//     friends JSON NOT NULL                -- 好友列表，JSON数组，每个元素是一个好友的账号和用户名
// );                                       -- { "account": "xxx", "muted": false } 不存储用户名


// CREATE TABLE friend_requests (
//     id INT PRIMARY KEY AUTO_INCREMENT,                         -- 主键ID，自动递增
//     sender VARCHAR(64) NOT NULL,                               -- 发起请求的用户账号
//     receiver VARCHAR(64) NOT NULL,                             -- 接收请求的用户账号
//     status ENUM('pending', 'accepted', 'rejected') NOT NULL    -- 当前状态（待处理 / 接受 / 拒绝）
//         DEFAULT 'pending',
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,              -- 请求的时间

//     INDEX idx_sender_receiver (sender, receiver),              -- 联合索引，便于查重、更新状态
//     INDEX idx_receiver_status (receiver, status)               -- 索引，便于查找所有待处理请求
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
        redis.expire(token_key, 3600);  // 1小时有效期 如果异常退出会在1个小时后过期

        redis.setex("online:" + account, 3600, "1");

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

// 注销帐号处理函数
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
        Redis redis("tcp://127.0.0.1:6379");

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

     













// 要删除所有和这个用户有关的表

















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
    
    redis.del("token:" + token);
    redis.del("online:" + account);

    send_json(fd, response);
}

// 退出帐号处理函数
void quit_account_msg(int fd, const json &request){
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
        Redis redis("tcp://127.0.0.1:6379");
        redis.del("token:" + token);

        redis.del("online:" + account);

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

// 显示用户名处理函数
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



// 改变用户名处理函数
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

// 修改密码处理函数
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
        auto redis = sw::redis::Redis("tcp://127.0.0.1:6379");
        
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

redis.del("token:" + token); // 删除 Redis key
redis.del("online:" + account);
    send_json(fd, response);
}


















































void show_friend_list_msg(int fd, const json& request) {
    json response;
    response["type"] = "show_friend_list";

    std::string token = request.value("token", "");
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
        auto stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT friends FROM friends WHERE account = ?"));
        stmt->setString(1, account);
        auto res = stmt->executeQuery();

        std::vector<json> friend_list;
        if (res->next()) {
            std::string friends_json = res->getString("friends");
            json friends = json::parse(friends_json); 

            Redis redis("tcp://127.0.0.1:6379");

            // 遍历好友，查询 Redis 是否在线
            for (const auto& f : friends) {
                std::string friend_account = f.value("account", "");
                // std::string friend_username = f.value("username", "");
                bool friend_is_muted = f.value("muted", "");

                bool is_online = redis.exists("online:" + friend_account);

                // 查用户名
                std::string friend_username;
                {
                    auto user_stmt = conn->prepareStatement(
                        "SELECT JSON_EXTRACT(info, '$.username') AS username FROM users WHERE JSON_EXTRACT(info, '$.account') = ?");
                    user_stmt->setString(1, friend_account);
                    auto user_res = user_stmt->executeQuery();
                    if (user_res->next()) {
                        friend_username = user_res->getString("username");
                        // username 可能带双引号，去掉

                        
                        // if (!friend_username.empty() && friend_username.front() == '"' && friend_username.back() == '"')
                        // friend_username = friend_username.substr(1, friend_username.size() - 2);
                    
                    
                    } else {
                        friend_username = "";
                    }
                }

                json friend_info;
                friend_info["account"] = friend_account;
                friend_info["username"] = friend_username;
                friend_info["online"] = is_online;
                friend_info["muted"] = friend_is_muted;

                friend_list.push_back(friend_info);
            }
        }

        response["status"] = "success";
        response["friends"] = friend_list;
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}

void mute_friend_msg(int fd, const json& request) {
    json response;
    response["type"] = "mute_friend";
    std::string token = request.value("token", "");
    std::string target_username = request.value("target_username", "");
    std::string user;

    if (!verify_token(token, user)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 找到目标账号,获取帐号
        std::string target_account;
        {
            auto stmt = conn->prepareStatement("SELECT info FROM users");
            auto res = stmt->executeQuery();
            while (res->next()) {
                json info = json::parse(res->getString("info"));
                if (info["username"] == target_username) {
                    target_account = info["account"];
                    break;
                }
            }
        }

        if (target_account.empty()) {
            response["status"] = "fail";
            response["msg"] = "User not found";
            send_json(fd, response);
            return;
        }

        // 获取当前用户的 friends 列
        auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
        stmt->setString(1, user);
        auto res = stmt->executeQuery();

        if (!res->next()) {
            response["status"] = "fail";
            response["msg"] = "Friend list not found";
            send_json(fd, response);
            return;
        }

        std::string friends_str = res->getString("friends");
        json friends_json = json::parse(friends_str);


        // 设置指定好友的 muted = true
        bool found = false;
        for (auto& f : friends_json) {
            if (f["account"] == target_account) {
                f["muted"] = true;
                found = true;
                break;
            }
        }

        if (!found) {
            response["status"] = "fail";
            response["msg"] = "Friend not found in friend list";
            send_json(fd, response);
            return;
        }

        // 更新数据库
        auto update_stmt = conn->prepareStatement("UPDATE friends SET friends = ? WHERE account = ?");
        update_stmt->setString(1, friends_json.dump());
        update_stmt->setString(2, user);
        update_stmt->execute();

        response["status"] = "success";
        response["msg"] = "muted successfully";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}

void unmute_friend_msg(int fd, const json& request) {
    json response;
    response["type"] = "unmute_friend";
    std::string token = request.value("token", "");
    std::string target_username = request.value("target_username", "");
    std::string user;

    if (!verify_token(token, user)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        std::string target_account;
        {
            auto stmt = conn->prepareStatement("SELECT info FROM users");
            auto res = stmt->executeQuery();
            while (res->next()) {
                json info = json::parse(res->getString("info"));
                if (info["username"] == target_username) {
                    target_account = info["account"];
                    break;
                }
            }
        }

        if (target_account.empty()) {
            response["status"] = "fail";
            response["msg"] = "User not found";
            send_json(fd, response);
            return;
        }

        auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
        stmt->setString(1, user);
        auto res = stmt->executeQuery();

        if (!res->next()) {
            response["status"] = "fail";
            response["msg"] = "Friend list not found";
            send_json(fd, response);
            return;
        }

        std::string friends_str = res->getString("friends");
        json friends_json = json::parse(friends_str);

        bool found = false;
        for (auto& f : friends_json) {
            if (f["account"] == target_account) {
                f["muted"] = false;
                found = true;
                break;
            }
        }

        if (!found) {
            response["status"] = "fail";
            response["msg"] = "Friend not found in friend list";
            send_json(fd, response);
            return;
        }

        auto update_stmt = conn->prepareStatement("UPDATE friends SET friends = ? WHERE account = ?");
        update_stmt->setString(1, friends_json.dump());
        update_stmt->setString(2, user);
        update_stmt->execute();

        response["status"] = "success";
        response["msg"] = "Unmuted successfully";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}

void remove_friend_msg(int fd, const json& request) {
    json response;
    response["type"] = "remove_friend";
    std::string token = request.value("token", "");
    std::string target_username = request.value("target_username", "");
    std::string user;

    if (!verify_token(token, user)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 查询目标账号
        auto info_stmt = conn->prepareStatement("SELECT info FROM users");
        auto info_res = info_stmt->executeQuery();

        std::string target_account;
        while (info_res->next()) {
            json info = json::parse(info_res->getString("info"));
            if (info["username"] == target_username) {
                target_account = info["account"];
                break;
            }
        }

        if (target_account.empty()) {
            response["status"] = "fail";
            response["msg"] = "User not found";
            send_json(fd, response);
            return;
        }


        auto remove = [&](const std::string& owner, const std::string& remove_account) {
            auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
            stmt->setString(1, owner);
            auto res = stmt->executeQuery();
            // 初始化一个 JSON 类型的空数组
            json fs = json::array();
            if (res->next()) fs = json::parse(res->getString("friends"));
            json new_fs = json::array();
            for (auto& f : fs) {
                if (f.value("account", "") != remove_account) 
                    new_fs.push_back(f); // 保留没有删除的好友
            }
            auto update = conn->prepareStatement("REPLACE INTO friends(account, friends) VALUES (?, ?)");
            update->setString(1, owner);
            update->setString(2, new_fs.dump());
            update->execute();
        };

        remove(user, target_account);
        remove(target_account, user);

        response["status"] = "success";
        response["msg"] = "Friend removed";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }
    send_json(fd, response);
}






// -- 1. 检查是否已发过好友请求
// SELECT id FROM friend_requests
// WHERE sender = 'alice' AND receiver = 'bob';

// -- 2. 更新请求状态为已接受
// UPDATE friend_requests
// SET status = 'accepted'
// WHERE sender = 'alice' AND receiver = 'bob';

// -- 3. 查询某用户所有未处理请求
// SELECT * FROM friend_requests
// WHERE receiver = 'bob' AND status = 'pending'
// ORDER BY timestamp DESC;





// CREATE TABLE friend_requests (
//     id INT PRIMARY KEY AUTO_INCREMENT,                         -- 主键ID，自动递增
//     sender VARCHAR(64) NOT NULL,                               -- 发起请求的用户账号
//     receiver VARCHAR(64) NOT NULL,                             -- 接收请求的用户账号
//     status ENUM('pending', 'accepted', 'rejected') NOT NULL    -- 当前状态（待处理 / 接受 / 拒绝）
//         DEFAULT 'pending',
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,              -- 请求的时间

//     INDEX idx_sender_receiver (sender, receiver),              -- 联合索引，便于查重、更新状态
//     INDEX idx_receiver_status (receiver, status)               -- 索引，便于查找所有待处理请求
// );


void add_friend_msg(int fd, const json& request) {
    json response;
    response["type"] = "add_friend";

    std::string token = request.value("token", "");
    std::string target_username = request.value("target_username", "");
    std::string sender;

    if (!verify_token(token, sender)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 查找对方账号
        std::string target_account;
        auto stmt = conn->prepareStatement(
            "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
            "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?");
        stmt->setString(1, target_username);
        auto res = stmt->executeQuery();
        if (res->next()) {
            target_account = res->getString("account");
        }

        if (target_account.empty()) {
            response["status"] = "fail";
            response["msg"] = "User not found";
            send_json(fd, response);
            return;
        }

        // 检查是否已是好友
        stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
        stmt->setString(1, sender);
        res = stmt->executeQuery();
        if (res->next()) {
            json friends = json::parse(res->getString("friends"));
            for (auto& f : friends) {
                if (f.value("account", "") == target_account) {
                    response["status"] = "fail";
                    response["msg"] = "Already friends";
                    send_json(fd, response);
                    return;
                }
            }
        }

        // 插入好友请求
        stmt = conn->prepareStatement(
            "INSERT INTO friend_requests(sender, receiver, status, timestamp) VALUES (?, ?, 'pending', NOW())");
        stmt->setString(1, sender);
        stmt->setString(2, target_account);
        stmt->execute();

        response["status"] = "success";
        response["msg"] = "Friend request sent";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}







void get_friend_requests_msg(int fd, const json& request) {
    json response;
    response["type"] = "get_friend_requests";

    std::string token = request.value("token", "");
    std::string receiver;

    if (!verify_token(token, receiver)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();
        auto stmt = conn->prepareStatement(
            "SELECT sender FROM friend_requests WHERE receiver = ? AND status = 'pending'");
        stmt->setString(1, receiver);
        auto res = stmt->executeQuery();

        json requests = json::array();

        while (res->next()) {
            std::string sender_account = res->getString("sender");

            auto uname_stmt = conn->prepareStatement(
                "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) AS username "
                "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) = ?");
            uname_stmt->setString(1, sender_account);
            auto uname_res = uname_stmt->executeQuery();

            std::string sender_username = "";
            if (uname_res->next()) {
                sender_username = uname_res->getString("username");
            }

            requests.push_back({
                {"account", sender_account},
                {"username", sender_username}
            });
        }

        response["status"] = "success";
        response["requests"] = requests;
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}

void handle_friend_request_msg(int fd, const json& request) {
    json response;
    response["type"] = "handle_friend_request";

    std::string token = request.value("token", "");
    std::string from_username = request.value("from_username", "");
    std::string action = request.value("action", ""); // "accept" or "reject"
    std::string receiver;

    if (!verify_token(token, receiver)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 查 sender 账号
        std::string sender;
        auto stmt = conn->prepareStatement(
            "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
            "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?");
        stmt->setString(1, from_username);
        auto res = stmt->executeQuery();
        if (res->next()) {
            sender = res->getString("account");
        }

        if (sender.empty()) {
            response["status"] = "fail";
            response["msg"] = "Sender not found";
            send_json(fd, response);
            return;
        }

        auto check_stmt = conn->prepareStatement(
            "SELECT status FROM friend_requests WHERE sender = ? AND receiver = ?");
        check_stmt->setString(1, sender);
        check_stmt->setString(2, receiver);
        auto check_res = check_stmt->executeQuery();

        if (!check_res->next() || check_res->getString("status") != "pending") {
            response["status"] = "fail";
            response["msg"] = "Invalid or already handled request";
            send_json(fd, response);
            return;
        }

        if (action == "accept") {
            // 更新请求状态
            auto update_stmt = conn->prepareStatement(
                "UPDATE friend_requests SET status = 'accepted' WHERE sender = ? AND receiver = ?");
            update_stmt->setString(1, sender);
            update_stmt->setString(2, receiver);
            update_stmt->execute();

            // 添加好友（只存账号）
            auto add_friend = [&](const std::string& owner, const std::string& other) {
                auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
                stmt->setString(1, owner);
                auto res = stmt->executeQuery();

                json friends = json::array();
                if (res->next()) {
                    friends = json::parse(res->getString("friends"));
                }

                for (auto& f : friends) {
                    if (f.value("account", "") == other) return; // 已存在
                }

                friends.push_back({{"account", other}, {"muted", false}});
                auto update = conn->prepareStatement(
                    "REPLACE INTO friends(account, friends) VALUES (?, ?)");
                update->setString(1, owner);
                update->setString(2, friends.dump());
                update->execute();
            };

            add_friend(receiver, sender);
            add_friend(sender, receiver);

            response["status"] = "success";
            response["msg"] = "Friend request accepted";
        } else if (action == "reject") {
            auto update_stmt = conn->prepareStatement(
                "UPDATE friend_requests SET status = 'rejected' WHERE sender = ? AND receiver = ?");
            update_stmt->setString(1, sender);
            update_stmt->setString(2, receiver);
            update_stmt->execute();

            response["status"] = "success";
            response["msg"] = "Friend request rejected";
        } else {
            response["status"] = "fail";
            response["msg"] = "Invalid action";
        }
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}


















    // if (!verify_token(token, account)) {
    //     response["type"] = "username_change";
    //     response["status"] = "error";
    //     response["msg"] = "Invalid or expired token";
    //     send_json(fd, response);
    //     return;
    // }

























































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


