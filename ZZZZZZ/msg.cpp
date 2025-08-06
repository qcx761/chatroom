// 未使用智能指针版本
#include "json.hpp"
#include "msg.hpp"

Redis redis("tcp://127.0.0.1:6379");

std::unordered_map<std::string, int> account_fd_map;
std::mutex fd_mutex;

// 用户表：users

// CREATE TABLE users (
//     id INT PRIMARY KEY AUTO_INCREMENT,  -- 主键，自动递增的整数ID
//     info JSON                           -- 一个JSON类型的字段，用来存储结构化的JSON数据
// );


// 好友表：friends

// CREATE TABLE friends (
//     account VARCHAR(64) PRIMARY KEY,     -- 当前用户账号，主键
//     friends JSON NOT NULL                -- 好友列表，JSON数组，每个元素是一个好友的账号和用户名
// );                                       -- { "account": "xxx", "muted": false } 不存储用户名


// 好友请求表：friend_requests

// CREATE TABLE friend_requests (
//     id INT PRIMARY KEY AUTO_INCREMENT,                         -- 主键ID，自动递增
//     sender VARCHAR(64) NOT NULL,                               -- 发起请求的用户账号
//     receiver VARCHAR(64) NOT NULL,                             -- 接收请求的用户账号
//     status ENUM('pending', 'accepted', 'rejected') NOT NULL    -- 当前状态（待处理 / 接受 / 拒绝）
//         DEFAULT 'pending',
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP               -- 请求的时间
// );


// //    INDEX idx_sender_receiver (sender, receiver),              -- 联合索引，便于查重、更新状态
// //    INDEX idx_receiver_status (receiver, status),              -- 索引，便于查找所有待处理请求

// 好友私聊

// CREATE TABLE messages (
//     id INT AUTO_INCREMENT PRIMARY KEY,
//     sender VARCHAR(64),
//     receiver VARCHAR(64),
//     content TEXT,
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
//     is_online BOOLEAN DEFAULT FALSE,
//     is_read BOOLEAN DEFAULT FALSE
// );

//通过account获取fd
int get_fd_by_account(const std::string& account) {
    std::lock_guard<std::mutex> lock(fd_mutex);  // 加锁保护
    auto it = account_fd_map.find(account);
    if (it != account_fd_map.end()) {
        return it->second;  // 找到返回fd
    }
    return -1;  // 没有找到
}

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
        // Redis redis("tcp://127.0.0.1:6379");
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
        // Redis redis("tcp://127.0.0.1:6379");
        std::string token = generate_token();
        std::string token_key = "token:" + token;

        json token_info = {
            {"account", user_info["account"]},
//可以不存储用户名
            {"username", user_info["username"]}
        };

        {
        std::lock_guard<std::mutex> lock(fd_mutex);
        account_fd_map[account] = fd;
        }

        redis.set(token_key, token_info.dump());
        redis.expire(token_key, 36000);  // 10小时有效期 如果异常退出会在10个小时后过期

        redis.setex("online:" + account, 36000, "1");

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
        // Redis redis("tcp://127.0.0.1:6379");

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

                // // 删除 users 表记录
                // auto del_stmt = std::unique_ptr<sql::PreparedStatement>(
                //     conn->prepareStatement("DELETE FROM users WHERE id = ?"));
                // del_stmt->setInt(1, id);
                // del_stmt->executeUpdate();

                // // 删除该账号在 friends 表中的记录（自己为主账号）
                // auto del_friends_stmt = std::unique_ptr<sql::PreparedStatement>(
                //     conn->prepareStatement("DELETE FROM friends WHERE account = ?"));
                // del_friends_stmt->setString(1, account);
                // del_friends_stmt->executeUpdate();

                // // 删除出现在他人好友列表中的该账号
                // auto get_all_stmt = std::unique_ptr<sql::PreparedStatement>(
                //     conn->prepareStatement("SELECT account, friends FROM friends"));
                // auto all_res = get_all_stmt->executeQuery();

                // while (all_res->next()) {
                //     std::string acc = all_res->getString("account");
                //     std::string friends_str = all_res->getString("friends");
                //     json friends = json::parse(friends_str);

                //     bool changed = false;
                //     json new_friends = json::array();
                //     for (auto& f : friends) {
                //         if (f.value("account", "") != account) {
                //             new_friends.push_back(f);
                //         } else {
                //             changed = true;
                //         }
                //     }

                //     if (changed) {
                //         auto update_stmt = std::unique_ptr<sql::PreparedStatement>(
                //             conn->prepareStatement("REPLACE INTO friends(account, friends) VALUES (?, ?)"));
                //         update_stmt->setString(1, acc);
                //         update_stmt->setString(2, new_friends.dump());
                //         update_stmt->execute();
                //     }
                // }

                // // 删除与该用户有关的好友请求记录
                // auto del_requests_stmt = std::unique_ptr<sql::PreparedStatement>(
                //     conn->prepareStatement("DELETE FROM friend_requests WHERE sender = ? OR receiver = ?"));
                // del_requests_stmt->setString(1, account);
                // del_requests_stmt->setString(2, account);
                // del_requests_stmt->executeUpdate();
// 所有表的注销
     



























// 要删除所有和这个用户有关的表























                // 删除成功
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

    {
        std::lock_guard<std::mutex> lock(fd_mutex);
        account_fd_map.erase(account);
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
        response["type"] = "quit_account";
        response["status"] = "error";
        response["msg"] = "Invalid or expired token";
        send_json(fd, response);
        return;
    }


    try {
        response["type"] = "quit_account";
        response["status"] = "success";
        response["msg"] = "Logged out";
    } catch (const std::exception &e) {
        response["type"] = "quit_account";
        response["status"] = "error";
        response["msg"] = e.what();
    }

    {
        std::lock_guard<std::mutex> lock(fd_mutex);
        account_fd_map.erase(account);
    }

    redis.del("token:" + token);
    redis.del("online:" + account);
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
            json info = json::parse(res->getString("info"));

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
    send_json(fd, response);
}

// 展示好友列表，包括在线状态和是否屏蔽
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

            // Redis redis("tcp://127.0.0.1:6379");

            // 遍历好友，查询 Redis 是否在线
            for (const auto& f : friends) {
                std::string friend_account = f.value("account", "");
                // std::string friend_username = f.value("username", "");
                // bool friend_is_muted = f.value("muted", "");
                // 默认值是 false，而不是空字符串

                bool friend_is_muted = f.value("muted", false);
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

// 屏蔽好友
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
                json info = json::parse(std::string(res->getString("info")));

                // json info = json::parse(res->getString("info"));
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

// 解除屏蔽好友
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
                json info = json::parse(std::string(res->getString("info")));

                // json info = json::parse(res->getString("info"));
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

// 删除好友
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
            json info = json::parse(std::string(info_res->getString("info")));
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

        // 判断是否为好友并删除
        auto remove = [&](const std::string& owner, const std::string& remove_account) -> bool {
            auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
            stmt->setString(1, owner);
            auto res = stmt->executeQuery();

            json fs = json::array();
            if (res->next())
                fs = json::parse(std::string(res->getString("friends")));

            bool found = false;
            json new_fs = json::array();
            for (auto& f : fs) {
                if (f.value("account", "") == remove_account) {
                    found = true;
                    continue; // 不加入新数组，即删除
                }
                new_fs.push_back(f);
            }

            if (found) {
                auto update = conn->prepareStatement("REPLACE INTO friends(account, friends) VALUES (?, ?)");
                update->setString(1, owner);
                update->setString(2, new_fs.dump());
                update->execute();
            }

            return found;
        };

        bool user_has_friend = remove(user, target_account);
        bool target_has_friend = remove(target_account, user);

        if (!user_has_friend) {
            response["status"] = "fail";
            response["msg"] = "The user is not your friend";
        } else {
            response["status"] = "success";
            response["msg"] = "Friend removed";
        }
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}

// 添加好友
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

        if (sender == target_account) {
            response["status"] = "fail";
            response["msg"] = "Cannot add yourself as a friend";
            send_json(fd, response);
            return;
        }

        // 检查是否已经是好友
        stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
        stmt->setString(1, sender);
        res = stmt->executeQuery();
        if (res->next()) {
            json friends = json::parse(std::string(res->getString("friends")));
            for (auto& f : friends) {
                if (f.value("account", "") == target_account) {
                    response["status"] = "fail";
                    response["msg"] = "Already friends";
                    send_json(fd, response);
                    return;
                }
            }
        }

        // 检查是否有历史请求（即使是 accepted 或 rejected）
        stmt = conn->prepareStatement(
            "SELECT id, status FROM friend_requests WHERE sender = ? AND receiver = ? ORDER BY id DESC");
        stmt->setString(1, sender);
        stmt->setString(2, target_account);
        res = stmt->executeQuery();

        bool handled = false;
        int latest_id = -1;
        std::string last_status;
        if (res->next()) {
            latest_id = res->getInt("id");
            last_status = res->getString("status");

            if (last_status == "pending") {
                // 更新时间
                stmt = conn->prepareStatement(
                    "UPDATE friend_requests SET timestamp = NOW() WHERE id = ?");
                stmt->setInt(1, latest_id);
                stmt->execute();
                response["status"] = "success";
                response["msg"] = "Friend request refreshed";
                send_json(fd, response);
                return;
            } else if (last_status == "accepted") {
                // 检查是否确实是好友（防止误删）
                stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
                stmt->setString(1, sender);
                auto res2 = stmt->executeQuery();
                if (res2->next()) {
                    json friends = json::parse(std::string(res2->getString("friends")));
                    for (auto& f : friends) {
                        if (f.value("account", "") == target_account) {
                            response["status"] = "fail";
                            response["msg"] = "Already friends";
                            send_json(fd, response);
                            return;
                        }
                    }
                }

                // 修改为 pending 再发
                stmt = conn->prepareStatement(
                    "UPDATE friend_requests SET status = 'pending', timestamp = NOW() WHERE id = ?");
                stmt->setInt(1, latest_id);
                stmt->execute();

                response["status"] = "success";
                response["msg"] = "Friend request re-sent";
                send_json(fd, response);
                return;
            }
        }

        // 新插入
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

// 拉取好友申请
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

        std::vector<json> requests;

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
        response["requests"] = json(requests);
        response["msg"] = "Get friend requests successful";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}

// 处理好友申请
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

        // 查找最新的 pending 请求
        auto check_stmt = conn->prepareStatement(
            "SELECT id FROM friend_requests WHERE sender = ? AND receiver = ? AND status = 'pending' ORDER BY id DESC");
        check_stmt->setString(1, sender);
        check_stmt->setString(2, receiver);
        auto check_res = check_stmt->executeQuery();

        if (!check_res->next()) {
            response["status"] = "fail";
            response["msg"] = "Invalid or already handled request";
            send_json(fd, response);
            return;
        }

        int request_id = check_res->getInt("id");

        if (action == "accept") {
            auto update_stmt = conn->prepareStatement(
                "UPDATE friend_requests SET status = 'accepted' WHERE id = ?");
            update_stmt->setInt(1, request_id);
            update_stmt->execute();

            auto add_friend = [&](const std::string& owner, const std::string& other) {
                auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
                stmt->setString(1, owner);
                auto res = stmt->executeQuery();

                json friends = json::array();
                if (res->next()) {
                    std::string friends_str(res->getString("friends"));
                    friends = json::parse(friends_str);
                }

                for (auto& f : friends) {
                    if (f.value("account", "") == other) return;
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
                "UPDATE friend_requests SET status = 'rejected' WHERE id = ?");
            update_stmt->setInt(1, request_id);
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

// 查询好友
void get_friend_info_msg(int fd, const json& request) {
    json response;
    response["type"] = "get_friend_info";

    std::string token = request.value("token", "");
    std::string friend_username = request.value("target_username", "");
    std::string user_account;

    if (!verify_token(token, user_account)) {

        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {

        auto conn = get_mysql_connection();

        // 好友用户名查账号
        std::string friend_account;
        {
            auto stmt = conn->prepareStatement(
                "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?");
            stmt->setString(1, friend_username);
            auto res = stmt->executeQuery();
            if (res->next()) {
                friend_account = res->getString("account");
            } else {
                response["status"] = "fail";
                response["msg"] = "Friend user not found";
                send_json(fd, response);
                return;
            }
        }

        // 确认好友关系
        bool is_friend = false;
        bool friend_is_muted = false;
        {
            auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
            stmt->setString(1, user_account);
            auto res = stmt->executeQuery();
            if (res->next()) {
                json friends = json::parse(std::string(res->getString("friends")));
                for (const auto& f : friends) {
                    if (f.value("account", "") == friend_account) {
                        is_friend = true;
                        friend_is_muted = f.value("muted", false);
                        break;
                    }
                }
            }
        }

        if (!is_friend) {
            response["status"] = "fail";
            response["msg"] = "This user is not your friend";
            send_json(fd, response);
            return;
        }
        bool is_online = redis.exists("online:" + friend_account);

        json friend_info;
        friend_info["account"] = friend_account;
        friend_info["username"] = friend_username;
        friend_info["muted"] = friend_is_muted;
        friend_info["online"] = is_online;

        response["status"] = "success";
        response["friend_info"] = friend_info;
        response["msg"] = "Friend info retrieved";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }
    send_json(fd, response);
}


























































// 查找历史记录
void get_private_history_msg(int fd, const json& request){
    json response;
    response["type"] = "get_private_history";

    std::string token = request.value("token", "");
    std::string target_username = request.value("target_username", "");
//     std::string before_time = request.value("before", "");  // 时间过滤
    int count = request.value("count", 10);

    std::string user_account;
    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 查找对方 account
        auto stmt = conn->prepareStatement(
            "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
            "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?");
        stmt->setString(1, target_username);
        auto res = stmt->executeQuery();

        std::string target_account;
        if (res->next()) {
            target_account = res->getString("account");
        } else {
            response["status"] = "fail";
            response["msg"] = "Target user not found";
            send_json(fd, response);
            return;
        }

        // 确认好友关系
        bool is_friend = false;
        bool friend_is_muted = false;
        {
            auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
            stmt->setString(1, user_account);
            auto res = stmt->executeQuery();
            if (res->next()) {
                json friends = json::parse(std::string(res->getString("friends")));
                for (const auto& f : friends) {
                    if (f.value("account", "") == target_account) {
                        is_friend = true;
                        friend_is_muted = f.value("muted", false);
                        break;
                    }
                }
            }
        }

        if (!is_friend) {
            response["status"] = "fail";
            response["msg"] = "This user is not your friend";
            send_json(fd, response);
            return;
        }

        // 查询消息记录
        std::string query =
        "SELECT sender, receiver, content, timestamp "
        "FROM messages "
        "WHERE ((sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?)) "
        "ORDER BY timestamp DESC LIMIT ?";
        // "ORDER BY id DESC LIMIT ?";

        auto msg_stmt = conn->prepareStatement(query);
        msg_stmt->setString(1, user_account);
        msg_stmt->setString(2, target_account);
        msg_stmt->setString(3, target_account);
        msg_stmt->setString(4, user_account);
        msg_stmt->setInt(5, count);

        auto msg_res = msg_stmt->executeQuery();

        json messages = json::array();
        while (msg_res->next()) {

            messages.push_back({
                {"from", msg_res->getString("sender")},
                {"to", msg_res->getString("receiver")},
                {"content", msg_res->getString("content")},
                {"timestamp", msg_res->getString("timestamp")}
            });
        }

        std::reverse(messages.begin(), messages.end()); // 按时间升序返回

        response["status"] = "success";
        response["msg"] = "Chat history fetched";
        response["messages"] = messages;

    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }
    send_json(fd, response);
}

// 发送私聊信息
void send_private_message_msg(int fd, const json& request){
    json response,response1;
    // 服务端返回发送信息
    response["type"] = "send_private_message";
    // 好友服务端接收信息
    response1["type"] = "receive_private_message";

    std::string token = request.value("token", "");
    std::string target_username = request.value("target_username", "");
    std::string message = request.value("message", "");

    std::string user_account;
    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    // 好友用户名查账号
    std::string target_account;
    {
        auto conn = get_mysql_connection();
        auto stmt = conn->prepareStatement(
            "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
            "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?");
        stmt->setString(1, target_username);
        auto res = stmt->executeQuery();
        if (res->next()) {
            target_account = res->getString("account");
        } else {
            response["status"] = "fail";
            response["msg"] = "Friend user not found";
            send_json(fd, response);
            return;
        }
    }

    // 查看是否屏蔽和好友
    bool is_friend = false;
    // 对象是否屏蔽你
    bool friend_is_muted = false;
    {
        auto conn = get_mysql_connection();
        auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
        stmt->setString(1, target_account);
        auto res = stmt->executeQuery();
        if (res->next()) {
            json friends = json::parse(std::string(res->getString("friends")));
            for (const auto& f : friends) {
                if (f.value("account", "") == user_account) {
                    is_friend = true;
                    friend_is_muted = f.value("muted", false);
                    break;
                }
            }
        }
    }

    if (!is_friend) {
        response["status"] = "fail";
        response["msg"] = "This user is not your friend";
        send_json(fd, response);
        return;
    }

    int target_fd=get_fd_by_account(target_account);
    bool is_online = redis.exists("online:" + target_account);

    // 储存到mysql
    try {
        auto conn = get_mysql_connection();
        auto stmt = conn->prepareStatement(
            "INSERT INTO messages (sender, receiver, content, is_online) VALUES (?, ?, ?, ?)");
        stmt->setString(1, user_account);
        stmt->setString(2, target_account);
        stmt->setString(3, message);
        stmt->setBoolean(4, is_online);
        stmt->execute();
    }catch (const std::exception& e) {
        std::cerr << "Error saving message: " << e.what() << std::endl;
    }

    if(is_online){
    //在线：调用 send() 发消息

        if(target_fd==-1){
            response["status"] = "fail";
            response["msg"] = "get fd fail";
            send_json(fd,response);
            return ;
        }

        response1["type"] = "receive_private_message";// 好友
        response1["from"] = user_account;
        response1["to"] = target_account;
        response1["message"] = message;
// 查看好友屏蔽
        response1["muted"] = friend_is_muted;



        // 推送消息给好友的客户端
        send_json(target_fd, response1);


    }else{
        ;
    //离线
    // 上线在哪里调用通知函数
    // 离线要怎么实现用户上线提示和发送信息
    // 上线的离线消息发送逻辑，遍历消息表输出未读消息
    }

    response["status"] = "success";
    send_json(fd,response);

    // 对好友发送信息，对自己客户端返回成功与否信息，保存到表中

    // response["type"] = "send_private_message";// 自己
    // response1["type"] = "receive_private_message";// 好友

    // 要实现fd的存储，发送到指定用户

    // 屏蔽信息不要推送 
}





void get_unread_private_messages_msg(int fd, const json& request){
    // 先判断好友再拉取得信息
    // 消息要变为已读发送完
    
    json response;
    response["type"] = "get_unread_private_messages";

    std::string token = request.value("token", "");
    std::string friend_username = request.value("target_username", "");
    std::string user_account;


    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 好友用户名查账号
        std::string friend_account;
        {
            auto stmt = conn->prepareStatement(
                "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?");
            stmt->setString(1, friend_username);
            auto res = stmt->executeQuery();
            if (res->next()) {
                friend_account = res->getString("account");
            } else {
                response["status"] = "fail";
                response["msg"] = "Friend user not found";
                send_json(fd, response);
                return;
            }
        }

        // 确认好友关系
        bool is_friend = false;
        bool friend_is_muted = false;
        {
            auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
            stmt->setString(1, user_account);
            auto res = stmt->executeQuery();
            if (res->next()) {
                json friends = json::parse(std::string(res->getString("friends")));
                for (const auto& f : friends) {
                    if (f.value("account", "") == friend_account) {
                        is_friend = true;
                        friend_is_muted = f.value("muted", false);
                        break;
                    }
                }
            }
        }

        if (!is_friend) {
            response["status"] = "fail";
            response["msg"] = "This user is not your friend";
            send_json(fd, response);
            return;
        }

        // 拉取未读取消息
        {
            auto stmt = conn->prepareStatement(
                "SELECT sender, content, timestamp FROM messages "
                "WHERE sender = ? AND receiver = ? AND is_online = FALSE AND is_read = FALSE");

            stmt->setString(1, friend_account);   // 好友的账号
            stmt->setString(2, user_account);     // 当前用户账号
            auto res = stmt->executeQuery();

            json messages = json::array();

            while (res->next()) {

                messages.push_back({
                    {"from", friend_account},
                    {"to", user_account},
                    {"content", res->getString("content")},
                    {"timestamp", res->getString("timestamp")}
                });
            }

           // std::reverse(messages.begin(), messages.end()); // 按时间升序返回

            // 我屏蔽好友就不能有离线
            if(friend_is_muted){
                messages.clear();  // 清空数组内容，保留类型为 array
            }

            response["status"] = "success";
            response["msg"] = "Unread private messages fetched";
            response["messages"] = messages;
        }

        {
        // 更新为已读
        auto update_stmt = conn->prepareStatement(
            "UPDATE messages SET is_read = TRUE WHERE receiver = ? AND is_read = FALSE");
        update_stmt->setString(1, user_account);
        update_stmt->execute();
        }
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }
    send_json(fd,response);
}




































// 群
// CREATE TABLE chat_groups (
//     group_id INT PRIMARY KEY AUTO_INCREMENT,        -- 群ID，自增
//     group_name VARCHAR(64) NOT NULL UNIQUE,         -- 群聊名，唯一
//     owner VARCHAR(64) NOT NULL,                     -- 群主账号
//     admins JSON NOT NULL DEFAULT '[]',              -- 管理员列表，允许为空
//     members JSON NOT NULL DEFAULT '[]',             -- 群成员列表，不能为空，默认空数组
//     created_at DATETIME DEFAULT CURRENT_TIMESTAMP   -- 创建时间
// );

// 群消息
// CREATE TABLE group_messages (
//     id INT PRIMARY KEY AUTO_INCREMENT,               -- 消息ID
//     group_id INT NOT NULL,                           -- 所属群ID
//     sender VARCHAR(64) NOT NULL,                     -- 发送者账号
//     content TEXT NOT NULL,                           -- 消息内容
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,    -- 发送时间
//     FOREIGN KEY (group_id) REFERENCES chat_groups(group_id) ON DELETE CASCADE
// );

// 成员在群信息
// CREATE TABLE group_members (
//     group_id INT NOT NULL COMMENT,                                     -- 群聊 ID，关联 chat_groups 表的主键
//     account VARCHAR(64) NOT NULL COMMENT,                              -- 用户账号，关联 users 表的主键或唯一字段
//     role ENUM('owner', 'admin', 'member') DEFAULT 'member' COMMENT,    -- 在群中的角色：群主、管理员或普通成员
// );

//     PRIMARY KEY (group_id, account),
//     FOREIGN KEY (group_id) REFERENCES chat_groups(id) ON DELETE CASCADE,
//     FOREIGN KEY (account) REFERENCES users(account) ON DELETE CASCADE

// CREATE TABLE group_requests (
//     id INT PRIMARY KEY AUTO_INCREMENT,                         -- 主键ID，自动递增
//     sender VARCHAR(64) NOT NULL,                               -- 发起申请的用户账号
//     group_id INT NOT NULL,                                     -- 要加入的群聊 ID
//     status ENUM('pending', 'accepted', 'rejected') NOT NULL    -- 当前状态（待处理 / 接受 / 拒绝）
//         DEFAULT 'pending',
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,              -- 申请发起时间
// );

//     FOREIGN KEY (group_id) REFERENCES chat_groups(group_id) ON DELETE CASCADE





void show_group_list_msg(int fd, const json& request) {
    json response;
    response["type"] = "show_group_list";

    std::string token = request.value("token", "");
    std::string user_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();
        auto stmt = conn->prepareStatement(
            "SELECT g.group_id, g.group_name, g.owner"
            "FROM chat_groups g "
            "JOIN group_members gm ON g.group_id = gm.group_id "
            "WHERE gm.account = ?"
        );
        stmt->setString(1, user_account);
        auto res = stmt->executeQuery();

        json groups = json::array();
        while (res->next()) {
            json group;
            group["group_id"] = res->getInt("group_id");
            group["group_name"] = res->getString("group_name");
            group["owner"] = res->getString("owner");
            groups.push_back(group);
        }

        response["status"] = "success";
        response["groups"] = groups;
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}





void join_group_msg(int fd, const json& request){
    json response;
    response["type"] = "join_group";

    std::string token = request.value("token", "");
    std::string user_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }


    try
    {
        auto conn = get_mysql_connection();

        int group_id=-1;
        auto stmt = conn->prepareStatement(
            "SELECT group_id FROM chat_groups WHERE group_name =?"
        );
        stmt->setString(1, group_name);
        res = stmt->executeQuery();
        if(res->next()){
            group_id=res->getInt("group_id");
        }else{
            response["status"] = "fail";
            response["msg"] = "Group not found";
            send_json(fd, response);
            return;
        }
        stmt
















        
    }catch(const std::exception& e){
        response["status"] = "error";
        response["msg"] = e.what();
    }
    send_json(fd,response);

}

void quit_group_msg(int fd, const json& request){
    ;
}

void set_group_admin_msg(int fd, const json& request){
    ;
}

void remove_group_admin_msg(int fd, const json& request){
    ;
}

void remove_group_member_msg(int fd, const json& request){
    ;
}   

void add_group_member_msg(int fd, const json& request){
    ;
}

void dismiss_group_msg(int fd, const json& request){
    ;
}

void get_unread_group_messages_msg(int fd, const json& request){
    ;
}

void get_group_history_msg(int fd, const json& request){
    ;
}

void send_group_message_msg(int fd, const json& request){
    ;
}

void get_group_requests_msg(int fd, const json& request){
    ;
}

void handle_group_request_msg(int fd, const json& request){
    ;
}




// void join_group(int sock,string token,sem_t& sem){
//     string group_name = readline_string("输入想加入的群聊名称 : ");
//     json j;
//     j["type"] = "join_group";
//     j["token"] = token;
//     j["group_name"] = group_name;
//     send_json(sock, j);
//     sem_wait(&sem);
//     waiting();
// }

// void quit_group(int sock,string token,sem_t& sem){
//     string group_name = readline_string("输入要退出的群聊名称 : ");
//     json j;
//     j["type"] = "quit_group";
//     j["token"] = token;
//     j["group_name"] = group_name;
//     send_json(sock, j);
//     sem_wait(&sem);
//     waiting();
// }

// void show_group_members(int sock,string token,sem_t& sem){
//     string group_name = readline_string("输入要查看成员的群聊名称 : ");
//     json j;
//     j["type"] = "show_group_members";
//     j["token"] = token;
//     j["group_name"] = group_name;
//     send_json(sock, j);
//     sem_wait(&sem);
//     waiting();
// }

// void create_group(int sock,string token,sem_t& sem){
//     string group_name = readline_string("请输入新建群聊名称 : ");
//     json j;
//     j["type"] = "create_group";
//     j["token"] = token;
//     j["group_name"] = group_name;
//     send_json(sock, j);
//     sem_wait(&sem);
//     waiting();
// }

// void set_group_admin(int sock,string token,sem_t& sem){
//     string group_name = readline_string("输入群聊名称 : ");
//     string target_user = readline_string("输入要设为管理员的用户名 : ");
//     json j;
//     j["type"] = "set_group_admin";
//     j["token"] = token;
//     j["group_name"] = group_name;
//     j["target_user"] = target_user;
//     send_json(sock, j);
//     sem_wait(&sem);
//     waiting();
// }

// void remove_group_admin(int sock,string token,sem_t& sem){
//     string group_name = readline_string("输入群聊名称 : ");
//     string target_user = readline_string("输入要移除管理员权限的用户名 : ");
//     json j;
//     j["type"] = "remove_group_admin";
//     j["token"] = token;
//     j["group_name"] = group_name;
//     j["target_user"] = target_user;
//     send_json(sock, j);
//     sem_wait(&sem);
//     waiting();
// }

// void remove_group_member(int sock,string token,sem_t& sem){
//     string group_name = readline_string("输入群聊名称 : ");
//     string target_user = readline_string("输入要移除的成员用户名 : ");
//     json j;
//     j["type"] = "remove_group_member";
//     j["token"] = token;
//     j["group_name"] = group_name;
//     j["target_user"] = target_user;
//     send_json(sock, j);
//     sem_wait(&sem);
//     waiting();
// }

// void add_group_member(int sock,string token,sem_t& sem){
//     string group_name = readline_string("输入群聊名称 : ");
//     string new_member = readline_string("输入新成员用户名 : ");
//     json j;
//     j["type"] = "add_group_member";
//     j["token"] = token;
//     j["group_name"] = group_name;
//     j["new_member"] = new_member;
//     send_json(sock, j);
//     sem_wait(&sem);
//     waiting();
// }

// void dismiss_group(int sock,string token,sem_t& sem){
//     string group_name = readline_string("输入要解散的群聊名称 : ");
//     json j;
//     j["type"] = "dismiss_group";
//     j["token"] = token;
//     j["group_name"] = group_name;
//     send_json(sock, j);
//     sem_wait(&sem);
//     waiting();
// }

// void send_group_message(int sock,string token,sem_t& sem){
// system("clear");
//     cout << "========== 群聊 ==========" << endl;

//     string target_group = readline_string("请输入想群聊的群名 : ");
//     if (target_group.empty()) {
//         cout << "[错误] 群名不能为空" << endl;
//         return;
//     }
//     // 设置当前群聊
//     current_chat_group = target_group;

//     // 拉取离线消息
//     json req;
//     req["type"] = "get_unread_group_messages";
//     req["token"] = token;
//     req["target_username"] = target_group;
//     send_json(sock, req);
//     sem_wait(&sem); // 等待信号量

//     cout << "进入 [" << target_group << "] 群" << endl;
//     cout << "提示："<< endl;
//     // cout << "- 输入消息并回车发送，输入 /exit 退出，/history [数量] 查看历史记录，/file [路径] 发送文件" << endl;
//     cout << "- 输入消息并回车发送" << endl;
//     cout << "- 输入 /history [数量] 查看历史记录" << endl;
//     cout << "- 输入 /file [路径] 传输文件" << endl;
//     cout << "- 输入 /exit 退出" << endl;
//     while (true) {
//         string message = readline_string("> ");
//         if (message == "/exit") {
//             // 退出当前群聊
//             current_chat_group = "";
//             cout << "[系统] 已退出群聊模式。" << endl;
//             break;
//         }

//         if (message.rfind("/history", 0) == 0) {
//             int count = 10;
//             std::istringstream iss_history(message);
//             string cmd;
//             iss_history >> cmd >> count;

//             json history;
//             history["type"]="get_group_history";
//             history["token"]=token;
//             history["target_username"]=target_group;
//             history["count"]=count;
//             send_json(sock, history);
//             sem_wait(&sem);  
//             continue;
//         }

//         if (message.rfind("/file", 0) == 0) {
//             string path;
//             std::istringstream iss_file(message);
//             string cmd;
//             iss_file >> cmd >> path;

//             if(path.empty()){
//                 cout << "未输入路径" << endl;
//                 cout << "[系统] 已退出文件传输模式。" << endl;
//                 break;
//             }

//             json file;

//             // file["type"]="send_file";
//             // file["token"]=token;
//             // file["target_username"]=target_group;
//             // file["path"]=path;
//             // send_json(sock, file);
//             // sem_wait(&sem);  











// // 文件传输
// // 记得通知





















//             continue;
//         }

//         json msg;
//         msg["type"]= "send_group_message";
//         msg["token"]=token;
//         msg["target_username"]=target_group;
//         msg["message"]=message;

//         send_json(sock, msg);
//         sem_wait(&sem);  

//     }
// }

// // 处理加群成员
// void getandhandle_group_request(int sock,string token,sem_t& sem){
//     system("clear");
//     string target_group = readline_string("请输入想处理成员添加的群名 : ");
//     std::cout << "请求列表" << std::endl;
//     json j;
//     j["type"]="get_group_requests";
//     j["token"]=token;
//     send_json(sock,j);
//     sem_wait(&sem);

//     if(global_group_requests.size()==0){
//         // flushInput();
//         waiting();
//         return;
//     }

//     string input = readline_string("输入处理编号(0 退出): ");
//     int choice = stoi(input);

//     // flushInput();
// // 要不要取消锁
//     std::string from_group;
// {
//     std::lock_guard<std::mutex> lock(group_requests_mutex);

//     if (choice <= 0 || choice > global_group_requests.size()) {
//         std::cout << "已取消处理。" << std::endl;
//         waiting();
//         return;
//     }

//     from_group = global_group_requests[choice - 1]["group"];
// }
//     string op = readline_string("你想如何处理 [" + from_group + "] 的请求？(a=接受, r=拒绝): ");
//     // flushInput();

//     std::string action;
//     if (op == "a" || op == "A") {
//         action = "accept";
//     } else if (op == "r" || op == "R") {
//         action = "reject";
//     } else {
//         std::cout << "无效操作，已取消处理。" << std::endl;
//         waiting();
//         return;
//     }

//     json m;
//     m["type"] = "handle_group_request";
//     m["token"] = token;
//     m["from_username"] = from_group;
//     m["action"] = action;
//     send_json(sock, m);
//     sem_wait(&sem);
//     waiting();
// }













































































void error_msg(int fd, const nlohmann::json &request){
    json response;
    response["type"] = "error";
    response["msg"] = "Unrecognized request type";
    send_json(fd, response);
}


