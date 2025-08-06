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


// 好友私聊：messages

// CREATE TABLE messages (
//     id INT AUTO_INCREMENT PRIMARY KEY,      -- 消息唯一ID，自增主键
//     sender VARCHAR(64),                     -- 发送者账号
//     receiver VARCHAR(64),                   -- 接收者账号
//     content TEXT,                           -- 消息内容，文本类型
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,  -- 消息发送时间，默认当前时间
//     is_online BOOLEAN DEFAULT FALSE,       -- 发送时接收者是否在线，默认为否
//     is_read BOOLEAN DEFAULT FALSE          -- 消息是否已读标志，默认为否
// );


// 群：chat_groups

// CREATE TABLE chat_groups (
//     group_id INT PRIMARY KEY AUTO_INCREMENT,        -- 群ID，自增
//     group_name VARCHAR(64) NOT NULL UNIQUE,         -- 群聊名，唯一
//     owner_account VARCHAR(64) NOT NULL              -- 群主账号
// );


// 群消息：group_messages

// CREATE TABLE group_messages (
//     id INT PRIMARY KEY AUTO_INCREMENT,               -- 消息ID
//     group_id INT NOT NULL,                           -- 所属群ID
//     sender VARCHAR(64) NOT NULL,                     -- 发送者账号
//     content TEXT NOT NULL,                           -- 消息内容
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP    -- 发送时间
// );


// 成员在群信息：group_members

// CREATE TABLE group_members (
//     group_id INT NOT NULL,                                     -- 群聊 ID，关联 chat_groups 表的主键
//     account VARCHAR(64) NOT NULL,                              -- 用户账号，关联 users 表的主键或唯一字段
//     role ENUM('owner', 'admin', 'member') DEFAULT 'member'    -- 在群中的角色：群主、管理员或普通成员
// );


// 加群请求：group_requests

// CREATE TABLE group_requests (
//     id INT PRIMARY KEY AUTO_INCREMENT,                         -- 主键ID，自动递增
//     sender VARCHAR(64) NOT NULL,                               -- 发起申请的用户账号
//     group_id INT NOT NULL,                                     -- 要加入的群聊 ID
//     status ENUM('pending', 'accepted', 'rejected') NOT NULL    -- 当前状态（待处理 / 接受 / 拒绝）
//         DEFAULT 'pending',
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP              -- 申请发起时间
// );


// 好友上次获取信息

// CREATE TABLE group_read_status (
//     group_id INT NOT NULL,
//     account VARCHAR(64) NOT NULL,
//     last_read_message_id INT DEFAULT 0,
//     PRIMARY KEY (group_id, account)
// );

// 通过 account 获取 fd
int get_fd_by_account(const std::string& account) {
    std::lock_guard<std::mutex> lock(fd_mutex);
    auto it = account_fd_map.find(account);
    if (it != account_fd_map.end()) {
        return it->second;
    }
    return -1;
}

// 获取 MySQL 连接
std::shared_ptr<sql::Connection> get_mysql_connection() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    auto conn = std::shared_ptr<sql::Connection>(driver->connect("tcp://127.0.0.1:3306", "qcx", "qcx761"));
    conn->setSchema("chatroom");
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
        std::string token_key = "token:" + token;
        auto val = redis.get(token_key);
        if (val) {
            json token_info = json::parse(*val);
            out_account = token_info.value("account", "");
            return true;
        } else {
            return false; // token不存在或者过期
        }
    } catch (const std::exception& e) {
        std::cerr << "Redis error: " << e.what() << std::endl;
        return false;
    }
}

// 注册处理函数
void sign_up_msg(int fd, const json &request) {
    json response;
    response["type"] = "sign_up";
    std::string account = request.value("account", "");
    std::string username = request.value("username", "");
    std::string password = request.value("password", "");

    if (account.empty() || username.empty() || password.empty()) {
        response["status"] = "error";
        response["msg"] = "Account, username or password cannot be empty";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 检查 account 是否存在，使用 MySQL JSON 查询（假设字段名 info）
        // 通过 conn->prepareStatement 预编译了一条 SQL 查询语句，语句里用 ? 作为占位符
        std::unique_ptr<sql::PreparedStatement> check_account_stmt(
            conn->prepareStatement("SELECT COUNT(*) FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
        check_account_stmt->setString(1, account);
        std::unique_ptr<sql::ResultSet> res_account(check_account_stmt->executeQuery());

        if (res_account->next() && res_account->getInt(1) > 0) {
            response["status"] = "fail";
            response["msg"] = "Account already exists";
        } else {
            std::unique_ptr<sql::PreparedStatement> check_username_stmt(
                conn->prepareStatement("SELECT COUNT(*) FROM users WHERE JSON_EXTRACT(info, '$.username') = ?"));
            check_username_stmt->setString(1, username);
            std::unique_ptr<sql::ResultSet> res_username(check_username_stmt->executeQuery());

            if (res_username->next() && res_username->getInt(1) > 0) {
                response["status"] = "fail";
                response["msg"] = "Username already exists";
            } else {
                json user_info = {
                    {"account", account},
                    {"username", username},
                    {"password", password}
                };

                std::unique_ptr<sql::PreparedStatement> insert_stmt(
                    conn->prepareStatement("INSERT INTO users (info) VALUES (?)"));
                insert_stmt->setString(1, user_info.dump());
                insert_stmt->executeUpdate();

                response["status"] = "success";
                response["msg"] = "Registered successfully";
            }
        }
    } catch (const std::exception &e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 登录处理函数
void log_in_msg(int fd, const json &request) {
    json response;
    response["type"] = "log_in";
    std::string account = request.value("account", "");
    std::string password = request.value("password", "");

    if (account.empty() || password.empty()) {
        response["status"] = "error";
        response["msg"] = "Account or password cannot be empty";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // JSON_EXTRACT(json_doc, path)，用于从 JSON 字段中提取你想要的某个值
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("SELECT info FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
        stmt->setString(1, account);
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        if (!res->next()) {
            response["status"] = "fail";
            response["msg"] = "Account not found";
            send_json(fd, response);
            return;
        }

        // 从 info 字段获取完整用户 JSON
        json user_info = json::parse(std::string(res->getString("info")));
        std::string stored_pass = user_info.value("password", "");

        if (stored_pass != password) {
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
            {"username", user_info["username"]}
        };

        {
            std::lock_guard<std::mutex> lock(fd_mutex);
            account_fd_map[account] = fd;
        }

        redis.set(token_key, token_info.dump());
        redis.expire(token_key, 36000); //token有十个小时有效期
        redis.setex("online:" + account, 36000, "1");

        response["status"] = "success";
        response["msg"] = "Login successful";
        response["token"] = token;
    } catch (const std::exception &e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 删除账户处理函数
void destory_account_msg(int fd, const json &request) {
    json response;
    response["type"] = "destory_account";
    std::string token = request.value("token", "");
    std::string account = request.value("account", "");
    std::string password = request.value("password", "");

    std::string redis_account;
    if (!verify_token(token, redis_account) || redis_account != account) {
        response["status"] = "error";
        response["msg"] = "Invalid or expired token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("SELECT id, info FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
        stmt->setString(1, account);
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        if (!res->next()) {
            response["status"] = "fail";
            response["msg"] = "Account not found";
        } else {
            json user_info = json::parse(std::string(res->getString("info")));
            if (user_info.value("password", "") != password) {
                response["status"] = "fail";
                response["msg"] = "Incorrect password";
            } else {
                int id = res->getInt("id");

                

                // 1. 删除 users 表记录
                {
                    auto del_stmt = std::unique_ptr<sql::PreparedStatement>(
                        conn->prepareStatement("DELETE FROM users WHERE id = ?"));
                    del_stmt->setInt(1, id);
                    del_stmt->executeUpdate();
                }

                // // 2. 删除 friends 表中本人为主账号的记录
                // {
                //     auto del_stmt = std::unique_ptr<sql::PreparedStatement>(
                //         conn->prepareStatement("DELETE FROM friends WHERE account = ?"));
                //     del_stmt->setString(1, account);
                //     del_stmt->executeUpdate();
                // }

                // // 3. 从其他用户的 friends 列表中移除自己
                // {
                //     auto get_all_stmt = std::unique_ptr<sql::PreparedStatement>(
                //         conn->prepareStatement("SELECT account, friends FROM friends"));
                //     auto all_res = std::unique_ptr<sql::ResultSet>(get_all_stmt->executeQuery());

                //     while (all_res->next()) {
                //         std::string acc = all_res->getString("account");
                //         json friends = json::parse(all_res->getString("friends"));
                //         json new_friends = json::array();
                //         bool changed = false;

                //         for (const auto &f : friends) {
                //             if (f.value("account", "") != account) {
                //                 new_friends.push_back(f);
                //             } else {
                //                 changed = true;
                //             }
                //         }

                //         if (changed) {
                //             auto update_stmt = std::unique_ptr<sql::PreparedStatement>(
                //                 conn->prepareStatement("REPLACE INTO friends(account, friends) VALUES (?, ?)"));
                //             update_stmt->setString(1, acc);
                //             update_stmt->setString(2, new_friends.dump());
                //             update_stmt->execute();
                //         }
                //     }
                // }

                // // 4. 删除自己相关的好友请求
                // {
                //     auto del_stmt = std::unique_ptr<sql::PreparedStatement>(
                //         conn->prepareStatement("DELETE FROM friend_requests WHERE sender = ? OR receiver = ?"));
                //     del_stmt->setString(1, account);
                //     del_stmt->setString(2, account);
                //     del_stmt->executeUpdate();
                // }

                // // 5. 删除私聊消息（发送或接收）
                // {
                //     auto del_stmt = std::unique_ptr<sql::PreparedStatement>(
                //         conn->prepareStatement("DELETE FROM messages WHERE sender = ? OR receiver = ?"));
                //     del_stmt->setString(1, account);
                //     del_stmt->setString(2, account);
                //     del_stmt->executeUpdate();
                // }

                // // 6. 删除本人创建的群聊（chat_groups 有 ON DELETE CASCADE）
                // {
                //     auto del_stmt = std::unique_ptr<sql::PreparedStatement>(
                //         conn->prepareStatement("DELETE FROM chat_groups WHERE owner_account = ?"));
                //     del_stmt->setString(1, account);
                //     del_stmt->executeUpdate();
                // }

                // // 7. 删除自己在群中的成员记录
                // {
                //     auto del_stmt = std::unique_ptr<sql::PreparedStatement>(
                //         conn->prepareStatement("DELETE FROM group_members WHERE account = ?"));
                //     del_stmt->setString(1, account);
                //     del_stmt->executeUpdate();
                // }

                // // 8. 删除自己发出的加群请求
                // {
                //     auto del_stmt = std::unique_ptr<sql::PreparedStatement>(
                //         conn->prepareStatement("DELETE FROM group_requests WHERE sender = ?"));
                //     del_stmt->setString(1, account);
                //     del_stmt->executeUpdate();
                // }

                
                
                
                
                
                
                
                
                
                
                
                

                
                
                
                
                
                {
                    std::lock_guard<std::mutex> lock(fd_mutex);
                    account_fd_map.erase(account);
                }
                
                redis.del("token:" + token);
                redis.del("online:" + account);

                response["status"] = "success";
                response["msg"] = "Account deleted";
            }
        }
    } catch (const std::exception &e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 退出帐号处理函数
void quit_account_msg(int fd, const json &request){
    json response;
    response["type"] = "quit_account";
    std::string token = request.value("token", "");
    std::string account;

    if (!verify_token(token, account)) {
        response["status"] = "error";
        response["msg"] = "Invalid or expired token";
        send_json(fd, response);
        return;
    }


    try {
        response["status"] = "success";
        response["msg"] = "Logged out";
    } catch (const std::exception &e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
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
    response["type"] = "username_view";
    std::string token = request.value("token", "");
    std::string account;

    if (!verify_token(token, account)) {
        response["status"] = "error";
        response["msg"] = "Invalid or expired token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("SELECT info FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
        stmt->setString(1, account);
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        if (res->next()) {
            json info = json::parse(std::string(res->getString("info")));
            

            response["status"] = "success";
            response["username"] = info["username"];
            response["msg"] = "View success";
        } else {
            response["status"] = "fail";
            response["msg"] = "User not found";
        }
    } catch (const std::exception &e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 改变用户名处理函数
void username_change_msg(int fd, const json &request){
    json response;
    response["type"] = "username_change";
    std::string token = request.value("token", "");
    std::string new_username = request.value("username", "");
    std::string account;

    if (!verify_token(token, account)) {
        response["status"] = "error";
        response["msg"] = "Invalid or expired token";
        send_json(fd, response);
        return;
    }

    if (new_username.empty()) {
        response["status"] = "error";
        response["msg"] = "New username cannot be empty";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 检查新用户名是否已存在
        std::unique_ptr<sql::PreparedStatement> check_stmt(
            conn->prepareStatement("SELECT COUNT(*) FROM users WHERE JSON_EXTRACT(info, '$.username') = ?"));
        check_stmt->setString(1, new_username);
        std::unique_ptr<sql::ResultSet> check_res(check_stmt->executeQuery());

        if (check_res->next() && check_res->getInt(1) > 0) {
            response["status"] = "fail";
            response["msg"] = "Username already exists";
            send_json(fd, response);
            return;
        }

        // 先查出当前用户的 info
        std::unique_ptr<sql::PreparedStatement> get_info_stmt(
            conn->prepareStatement("SELECT id, info FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
        get_info_stmt->setString(1, account);
        std::unique_ptr<sql::ResultSet> info_res(get_info_stmt->executeQuery());

        if (!info_res->next()) {
            response["status"] = "fail";
            response["msg"] = "User not found";
            send_json(fd, response);
            return;
        }

        int id = info_res->getInt("id");
        json user_info = json::parse(std::string(info_res->getString("info")));
        user_info["username"] = new_username;

        // 更新数据库
        std::unique_ptr<sql::PreparedStatement> update_stmt(
            conn->prepareStatement("UPDATE users SET info = ? WHERE id = ?"));
        update_stmt->setString(1, user_info.dump());
        update_stmt->setInt(2, id);
        update_stmt->executeUpdate();

        // 更新 redis token 信息中的 username
        std::string token_key = "token:" + token;
        auto val = redis.get(token_key);
        if (val) {
            json token_info = json::parse(*val);
            token_info["username"] = new_username;
            redis.set(token_key, token_info.dump());
            redis.expire(token_key, 36000);
        }

        response["status"] = "success";
        response["msg"] = "Username updated successfully";

    } catch (const std::exception &e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 修改密码处理函数
void password_change_msg(int fd, const json &request) {
    json response;
    response["type"] = "password_change";
    std::string token = request.value("token", "");
    std::string old_pass = request.value("old_password", "");
    std::string new_pass = request.value("new_password", "");
    std::string account;

    if (!verify_token(token, account)) {
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
        auto res = std::unique_ptr<sql::ResultSet>(select_stmt->executeQuery());

        if (res->next()) {
            int id = res->getInt("id");
            json info = json::parse(std::string(res->getString("info")));

            if (info["password"] != old_pass) {
                response["status"] = "fail";
                response["msg"] = "Old password incorrect";
            } else {
                info["password"] = new_pass;
                auto update_stmt = std::unique_ptr<sql::PreparedStatement>(
                    conn->prepareStatement("UPDATE users SET info = ? WHERE id = ?"));
                update_stmt->setString(1, info.dump());
                update_stmt->setInt(2, id);
                update_stmt->executeUpdate();

                response["status"] = "success";
                response["msg"] = "Password changed";
            }
        } else {
            response["status"] = "fail";
            response["msg"] = "Account not found";
        }
    } catch (const std::exception &e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();

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
        auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

        std::vector<json> friend_list;
        if (res->next()) {
            std::string friends_json = res->getString("friends");
            json friends = json::parse(friends_json);

            for (const auto& f : friends) {
                std::string friend_account = f.value("account", "");
                bool friend_is_muted = f.value("muted", false);
                bool is_online = redis.exists("online:" + friend_account);

                std::string friend_username;
                {
                    auto user_stmt = std::unique_ptr<sql::PreparedStatement>(
                        conn->prepareStatement(
                            "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) AS username "
                            "FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
                    user_stmt->setString(1, friend_account);
                    auto user_res = std::unique_ptr<sql::ResultSet>(user_stmt->executeQuery());
                    if (user_res->next()) {
                        friend_username = user_res->getString("username");
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
        response["msg"] = std::string("Exception: ") + e.what();
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

        // 查询目标账号（通过用户名）
        std::string target_account;
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                    "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?"));
            stmt->setString(1, target_username);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                target_account = res->getString("account");
            }
        }

        if (target_account.empty()) {
            response["status"] = "fail";
            response["msg"] = "User not found";
            send_json(fd, response);
            return;
        }

        // 获取当前用户的 friends 列
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("SELECT friends FROM friends WHERE account = ?"));
        stmt->setString(1, user);
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

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
        std::unique_ptr<sql::PreparedStatement> update_stmt(
            conn->prepareStatement("UPDATE friends SET friends = ? WHERE account = ?"));
        update_stmt->setString(1, friends_json.dump());
        update_stmt->setString(2, user);
        update_stmt->executeUpdate();

        response["status"] = "success";
        response["msg"] = "Muted successfully";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 取消屏蔽好友（通过好友用户名）
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

        // 通过用户名查找目标账号
        std::string target_account;
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                    "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?"));
            stmt->setString(1, target_username);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                target_account = res->getString("account");
            }
        }

        if (target_account.empty()) {
            response["status"] = "fail";
            response["msg"] = "User not found";
            send_json(fd, response);
            return;
        }

        // 查询当前用户的好友列表
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("SELECT friends FROM friends WHERE account = ?"));
        stmt->setString(1, user);
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        if (!res->next()) {
            response["status"] = "fail";
            response["msg"] = "Friend list not found";
            send_json(fd, response);
            return;
        }

        json friends_info = json::parse(std::string(res->getString("friends")));

        // 设置 muted = false
        bool found = false;
        for (auto& f : friends_info) {
            if (f["account"] == target_account) {
                f["muted"] = false;
                found = true;
                break;
            }
        }

        if (!found) {
            response["status"] = "fail";
            response["msg"] = "Friend not found in list";
            send_json(fd, response);
            return;
        }

        // 更新好友列表
        std::unique_ptr<sql::PreparedStatement> update_stmt(
            conn->prepareStatement("UPDATE friends SET friends = ? WHERE account = ?"));
        update_stmt->setString(1, friends_info.dump());
        update_stmt->setString(2, user);
        update_stmt->executeUpdate();

        response["status"] = "success";
        response["msg"] = "Unmuted successfully";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
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

        // 用 unique_ptr 包装 Statement 和 ResultSet，确保自动释放
        auto info_stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement("SELECT info FROM users"));
        auto info_res = std::unique_ptr<sql::ResultSet>(info_stmt->executeQuery());

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
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement("SELECT friends FROM friends WHERE account = ?"));
            stmt->setString(1, owner);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

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
                auto update = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement("REPLACE INTO friends(account, friends) VALUES (?, ?)"));
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
        response["msg"] = std::string("Exception: ") + e.what();
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

        // 查找目标用户账号
        std::string target_account;
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                    "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?"));
            stmt->setString(1, target_username);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                target_account = res->getString("account");
            }
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
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT friends FROM friends WHERE account = ?"));
            stmt->setString(1, sender);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                auto friends_str = std::string(res->getString("friends"));
                if (!friends_str.empty()) {
                    json friends = json::parse(friends_str);
                    for (const auto& f : friends) {
                        if (f.value("account", "") == target_account) {
                            response["status"] = "fail";
                            response["msg"] = "Already friends";
                            send_json(fd, response);
                            return;
                        }
                    }
                }
            }
        }

        // 检查是否已有请求
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "SELECT id, status FROM friend_requests WHERE sender = ? AND receiver = ? ORDER BY id DESC LIMIT 1"));
            stmt->setString(1, sender);
            stmt->setString(2, target_account);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                int request_id = res->getInt("id");
                std::string status = res->getString("status");

                if (status == "pending") {
                    std::unique_ptr<sql::PreparedStatement> update_stmt(
                        conn->prepareStatement("UPDATE friend_requests SET timestamp = NOW() WHERE id = ?"));
                    update_stmt->setInt(1, request_id);
                    update_stmt->execute();

                    response["status"] = "success";
                    response["msg"] = "Friend request refreshed";
                    send_json(fd, response);
                    return;
                } else if (status == "accepted") {
                    // 再次确认好友关系，防止误删
                    std::unique_ptr<sql::PreparedStatement> stmt_check(
                        conn->prepareStatement("SELECT friends FROM friends WHERE account = ?"));
                    stmt_check->setString(1, sender);
                    std::unique_ptr<sql::ResultSet> res_check(stmt_check->executeQuery());
                    bool is_friend = false;
                    if (res_check->next()) {
                        auto friends_str = std::string(res_check->getString("friends"));
                        if (!friends_str.empty()) {
                            json friends = json::parse(friends_str);
                            for (const auto& f : friends) {
                                if (f.value("account", "") == target_account) {
                                    is_friend = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (is_friend) {
                        response["status"] = "fail";
                        response["msg"] = "Already friends";
                        send_json(fd, response);
                        return;
                    }

                    std::unique_ptr<sql::PreparedStatement> update_stmt(
                        conn->prepareStatement(
                            "UPDATE friend_requests SET status = 'pending', timestamp = NOW() WHERE id = ?"));
                    update_stmt->setInt(1, request_id);
                    update_stmt->execute();

                    response["status"] = "success";
                    response["msg"] = "Friend request re-sent";
                    send_json(fd, response);
                    return;
                } else if (status == "rejected") {
                    // 重新发起请求
                    std::unique_ptr<sql::PreparedStatement> update_stmt(
                        conn->prepareStatement(
                            "UPDATE friend_requests SET status = 'pending', timestamp = NOW() WHERE id = ?"));
                    update_stmt->setInt(1, request_id);
                    update_stmt->execute();

                    response["status"] = "success";
                    response["msg"] = "Friend request re-sent";
                    send_json(fd, response);
                    return;
                }
            }
        }

        // 插入新请求
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "INSERT INTO friend_requests(sender, receiver, status, timestamp) VALUES (?, ?, 'pending', NOW())"));
            stmt->setString(1, sender);
            stmt->setString(2, target_account);
            stmt->execute();
        }

        response["status"] = "success";
        response["msg"] = "Friend request sent";

    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
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
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "SELECT sender FROM friend_requests WHERE receiver = ? AND status = 'pending'"));
        stmt->setString(1, receiver);
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        std::vector<json> requests;

        while (res->next()) {
            std::string sender_account = res->getString("sender");

            std::string sender_username;
            {
                std::unique_ptr<sql::PreparedStatement> uname_stmt(
                    conn->prepareStatement(
                        "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) AS username FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) = ?"));
                uname_stmt->setString(1, sender_account);
                std::unique_ptr<sql::ResultSet> uname_res(uname_stmt->executeQuery());

                if (uname_res->next()) {
                    sender_username = uname_res->getString("username");
                }
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
        response["msg"] = std::string("Exception: ") + e.what();
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
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?"));
            stmt->setString(1, from_username);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                sender = res->getString("account");
            }
        }

        if (sender.empty()) {
            response["status"] = "fail";
            response["msg"] = "Sender not found";
            send_json(fd, response);
            return;
        }

        // 找最新的pending请求
        int request_id = -1;
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "SELECT id FROM friend_requests WHERE sender = ? AND receiver = ? AND status = 'pending' ORDER BY id DESC LIMIT 1"));
            stmt->setString(1, sender);
            stmt->setString(2, receiver);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

            if (res->next()) {
                request_id = res->getInt("id");
            } else {
                response["status"] = "fail";
                response["msg"] = "Invalid or already handled request";
                send_json(fd, response);
                return;
            }
        }

        if (action == "accept") {
            // 更新请求状态
            {
                std::unique_ptr<sql::PreparedStatement> stmt(
                    conn->prepareStatement(
                        "UPDATE friend_requests SET status = 'accepted' WHERE id = ?"));
                stmt->setInt(1, request_id);
                stmt->executeUpdate();
            }

            // 双方好友表更新
            auto update_friends = [&](const std::string& acc1, const std::string& acc2) {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT friends FROM friends WHERE account = ?"));
            stmt->setString(1, acc1);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

            json friends_json = json::array();
            bool exists_in_table = false;

            if (res->next()) {
                exists_in_table = true;
                auto f_str = std::string(res->getString("friends"));
                if (!f_str.empty()) {
                    friends_json = json::parse(f_str);
                }
            }

            bool already_exists = false;
            for (const auto& f : friends_json) {
                if (f.value("account", "") == acc2) {
                    already_exists = true;
                    break;
                }
            }

            if (!already_exists) {
                friends_json.push_back({
                    {"account", acc2},
                    {"muted", false}
                });

                if (exists_in_table) {
                    std::unique_ptr<sql::PreparedStatement> update_stmt(
                        conn->prepareStatement("UPDATE friends SET friends = ? WHERE account = ?"));
                    update_stmt->setString(1, friends_json.dump());
                    update_stmt->setString(2, acc1);
                    update_stmt->executeUpdate();
                } else {
                    std::unique_ptr<sql::PreparedStatement> insert_stmt(
                        conn->prepareStatement("INSERT INTO friends (account, friends) VALUES (?, ?)"));
                    insert_stmt->setString(1, acc1);
                    insert_stmt->setString(2, friends_json.dump());
                    insert_stmt->executeUpdate();
                }
            }
        };

            update_friends(receiver, sender);
            update_friends(sender, receiver);

            response["status"] = "success";
            response["msg"] = "Friend request accepted";
        } else if (action == "reject") {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "UPDATE friend_requests SET status = 'rejected' WHERE id = ?"));
            stmt->setInt(1, request_id);
            stmt->executeUpdate();

            response["status"] = "success";
            response["msg"] = "Friend request rejected";
        } else {
            response["status"] = "fail";
            response["msg"] = "Invalid action";
        }
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
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
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                    "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?"));
            stmt->setString(1, friend_username);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

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
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT friends FROM friends WHERE account = ?"));
            stmt->setString(1, user_account);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

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
        response["msg"] = std::string("Exception: ") + e.what();
    }
    send_json(fd, response);
}






















































// 查找历史记录
void get_private_history_msg(int fd, const json& request) {
    json response;
    response["type"] = "get_private_history";
    std::string token = request.value("token", "");
    std::string target_username = request.value("target_username", "");
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

        // 查找对方账号
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?"));
        stmt->setString(1, target_username);
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        std::string target_account;
        if (res->next()) {
            target_account = res->getString("account");
        } else {
            response["status"] = "fail";
            response["msg"] = "Target user not found";
            send_json(fd, response);
            return;
        }

        // 确认好友关系且判断屏蔽（自己是否屏蔽对方）
        bool is_friend = false;
        bool user_muted_target = false;
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT friends FROM friends WHERE account = ?"));
            stmt->setString(1, user_account);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                json friends = json::parse(std::string(res->getString("friends")));
                for (const auto& f : friends) {
                    if (f.value("account", "") == target_account) {
                        is_friend = true;
                        user_muted_target = f.value("muted", false);
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

        if (user_muted_target) {
            response["status"] = "fail";
            response["msg"] = "You have muted this user";
            send_json(fd, response);
            return;
        }

        // 查询消息记录
        std::unique_ptr<sql::PreparedStatement> msg_stmt(
            conn->prepareStatement(
                "SELECT sender, receiver, content, timestamp "
                "FROM messages "
                "WHERE ((sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?)) "
                "ORDER BY timestamp DESC LIMIT ?"));
        msg_stmt->setString(1, user_account);
        msg_stmt->setString(2, target_account);
        msg_stmt->setString(3, target_account);
        msg_stmt->setString(4, user_account);
        msg_stmt->setInt(5, count);

        std::unique_ptr<sql::ResultSet> msg_res(msg_stmt->executeQuery());

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
        response["msg"] = std::string("Exception: ") + e.what();
    }
    send_json(fd, response);
}

// 发送私聊信息
void send_private_message_msg(int fd, const json& request) {
    json response, response1;
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

    std::string target_account;
    try {
        auto conn = get_mysql_connection();

        // 查找对方账号
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                    "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?"));
            stmt->setString(1, target_username);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                target_account = res->getString("account");
            } else {
                response["status"] = "fail";
                response["msg"] = "Friend user not found";
                send_json(fd, response);
                return;
            }
        }

        // 判断自己是否为对方好友（自己是否在对方好友列表中）
        bool is_friend = false;
        bool target_muted_user = false;
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT friends FROM friends WHERE account = ?"));
            stmt->setString(1, target_account);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                json friends = json::parse(std::string(res->getString("friends")));
                for (const auto& f : friends) {
                    if (f.value("account", "") == user_account) {
                        is_friend = true;
                        target_muted_user = f.value("muted", false);
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

        if (target_muted_user) {
            response["status"] = "fail";
            response["msg"] = "You are muted by the target user";
            send_json(fd, response);
            return;
        }

        // 储存消息到mysql
        bool is_online = redis.exists("online:" + target_account);
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "INSERT INTO messages (sender, receiver, content, is_online, is_read) VALUES (?, ?, ?, ?, FALSE)"));
            stmt->setString(1, user_account);
            stmt->setString(2, target_account);
            stmt->setString(3, message);
            stmt->setBoolean(4, is_online);
            stmt->execute();
        }

        // 在线则推送消息
        if (is_online) {
            int target_fd = get_fd_by_account(target_account);
            if (target_fd == -1) {
                response["status"] = "fail";
                response["msg"] = "Failed to get target user fd";
                send_json(fd, response);
                return;
            }

            response1["from"] = user_account;
            response1["to"] = target_account;
            response1["message"] = message;
            response1["muted"] = target_muted_user;

            send_json(target_fd, response1);
        }else{
                    ;
        //离线
        // 上线在哪里调用通知函数
        // 离线要怎么实现用户上线提示和发送信息
        // 上线的离线消息发送逻辑，遍历消息表输出未读消息
        }

        response["status"] = "success";
        send_json(fd, response);

    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
        send_json(fd, response);
    }
}

// 获取未读私聊消息
void get_unread_private_messages_msg(int fd, const json& request) {
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
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                    "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?"));
            stmt->setString(1, friend_username);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                friend_account = res->getString("account");
            } else {
                response["status"] = "fail";
                response["msg"] = "Friend user not found";
                send_json(fd, response);
                return;
            }
        }

        // 确认好友关系且判断自己是否屏蔽好友
        bool is_friend = false;
        bool user_muted_friend = false;
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT friends FROM friends WHERE account = ?"));
            stmt->setString(1, user_account);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                json friends = json::parse(std::string(res->getString("friends")));
                for (const auto& f : friends) {
                    if (f.value("account", "") == friend_account) {
                        is_friend = true;
                        user_muted_friend = f.value("muted", false);
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

        if (user_muted_friend) {
            response["status"] = "success";
            response["msg"] = "You have muted this user";
            response["messages"] = json::array();
            send_json(fd, response);
            return;
        }

        // 拉取未读消息
        json messages = json::array();
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "SELECT sender, content, timestamp FROM messages "
                    "WHERE sender = ? AND receiver = ? AND is_online = FALSE AND is_read = FALSE "
                    "ORDER BY timestamp ASC"));
            stmt->setString(1, friend_account);
            stmt->setString(2, user_account);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

            while (res->next()) {
                messages.push_back({
                    {"from", friend_account},
                    {"to", user_account},
                    {"content", res->getString("content")},
                    {"timestamp", res->getString("timestamp")}
                });
            }
        }

        // 更新为已读
        {
            std::unique_ptr<sql::PreparedStatement> update_stmt(
                conn->prepareStatement(
                    "UPDATE messages SET is_read = TRUE WHERE sender = ? AND receiver = ? AND is_read = FALSE"));
            update_stmt->setString(1, friend_account);
            update_stmt->setString(2, user_account);
            update_stmt->execute();
        }

        response["status"] = "success";
        response["msg"] = "Unread private messages fetched";
        response["messages"] = messages;

    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }
    send_json(fd, response);
}






































































// 显示加入的群
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

        // 通过帐号查找群id并通过群id查找群名字
        auto stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement(
                "SELECT g.group_id, g.group_name, g.owner_account, gm.role "
                "FROM chat_groups g "
                "JOIN group_members gm ON g.group_id = gm.group_id "
                "WHERE gm.account = ?"
            )
        );
        stmt->setString(1, user_account);

        auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

        json groups = json::array();
        while (res->next()) {
            json group;
            group["group_id"] = res->getInt("group_id");
            group["group_name"] = res->getString("group_name");
            group["owner"] = res->getString("owner_account");
            group["role"] = res->getString("role");
            groups.push_back(group);
        }

        response["status"] = "success";
        response["groups"] = groups;
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }
    send_json(fd, response);
}

// 申请群
void join_group_msg(int fd, const json& request) {
    json response;
    response["type"] = "join_group";
    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string sender_account;

    if (!verify_token(token, sender_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 查询 group_id
        int group_id = -1;
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT group_id FROM chat_groups WHERE group_name = ?"));
            stmt->setString(1, group_name);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                group_id = res->getInt("group_id");
            } else {
                response["status"] = "fail";
                response["msg"] = "Group not found";
                send_json(fd, response);
                return;
            }
        }

        // 检查是否已经在群中
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT 1 FROM group_members WHERE group_id = ? AND account = ?"));
            stmt->setInt(1, group_id);
            stmt->setString(2, sender_account);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                response["status"] = "fail";
                response["msg"] = "Already in the group";
                send_json(fd, response);
                return;
            }
        }

        // 检查是否已有申请
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "SELECT id, status FROM group_requests WHERE sender = ? AND group_id = ? ORDER BY id DESC LIMIT 1"));
            stmt->setString(1, sender_account);
            stmt->setInt(2, group_id);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

            if (res->next()) {
                int request_id = res->getInt("id");
                std::string status = res->getString("status");

                if (status == "pending") {
                    std::unique_ptr<sql::PreparedStatement> update_stmt(
                        conn->prepareStatement("UPDATE group_requests SET timestamp = NOW() WHERE id = ?"));
                    update_stmt->setInt(1, request_id);
                    update_stmt->execute();

                    response["status"] = "success";
                    response["msg"] = "Group join request refreshed";
                    send_json(fd, response);
                    return;
                } else {
                    std::unique_ptr<sql::PreparedStatement> update_stmt(
                        conn->prepareStatement(
                            "UPDATE group_requests SET status = 'pending', timestamp = NOW() WHERE id = ?"));
                    update_stmt->setInt(1, request_id);
                    update_stmt->execute();

                    response["status"] = "success";
                    response["msg"] = "Group join request re-sent";
                    send_json(fd, response);
                    return;
                }
            }
        }

        // 插入新申请
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "INSERT INTO group_requests(sender, group_id, status, timestamp) VALUES (?, ?, 'pending', NOW())"));
            stmt->setString(1, sender_account);
            stmt->setInt(2, group_id);
            stmt->execute();
        }

        response["status"] = "success";
        response["msg"] = "Group join request sent";

    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 退出群
void quit_group_msg(int fd, const json& request){
    json response;
    response["type"] = "quit_group";
    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name","");
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
        auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
            "SELECT group_id, owner_account FROM chat_groups WHERE group_name = ?"
        ));
        stmt->setString(1, group_name);
        auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

        int group_id = -1;
        std::string owner_account;
        if(res->next()){
            group_id = res->getInt("group_id");
            owner_account = res->getString("owner_account");
        }else{
            response["status"] = "fail";
            response["msg"] = "Group not found";
            send_json(fd,response);
            return;
        }

        if(owner_account == user_account){
            response["status"] = "fail";
            response["msg"] = "Owner can not quit group";
            send_json(fd,response);
            return;
        }

        auto del_stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
            "DELETE FROM group_members WHERE group_id = ? AND account = ?"
        ));
        del_stmt->setInt(1, group_id);
        del_stmt->setString(2, user_account);
        del_stmt->execute();

        response["status"] = "success";
        response["msg"] = "Quit group success";
    }catch(const std::exception& e)
    {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }
    send_json(fd,response);
}

// 显示群成员
void show_group_members_msg(int fd, const json& request) {
    json response;
    response["type"] = "show_group_members";
    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string user_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 获取 group_id
        int group_id = -1;
        {
            auto stmt_id = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("SELECT group_id FROM chat_groups WHERE group_name = ?"));
            stmt_id->setString(1, group_name);
            auto res_id = std::unique_ptr<sql::ResultSet>(stmt_id->executeQuery());

            if (res_id->next()) {
                group_id = res_id->getInt("group_id");
            } else {
                response["status"] = "fail";
                response["msg"] = "Group not found";
                send_json(fd, response);
                return;
            }
        }

        // 验证请求者是否是该群成员
        {
            auto stmt_check = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("SELECT 1 FROM group_members WHERE group_id = ? AND account = ?"));
            stmt_check->setInt(1, group_id);
            stmt_check->setString(2, user_account);
            auto res_check = std::unique_ptr<sql::ResultSet>(stmt_check->executeQuery());

            if (!res_check->next()) {
                response["status"] = "fail";
                response["msg"] = "You are not a member of this group";
                send_json(fd, response);
                return;
            }
        }

        // 查询成员
        auto stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement(
                "SELECT gm.account, gm.role, "
                "       JSON_UNQUOTE(JSON_EXTRACT(u.info, '$.username')) AS username "
                "FROM group_members gm "
                "JOIN users u ON gm.account = JSON_UNQUOTE(JSON_EXTRACT(u.info, '$.account')) "
                "WHERE gm.group_id = ?"
            ));
        stmt->setInt(1, group_id);
        auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());


        json members = json::array();
        while (res->next()) {
            json member;
            member["name"] = res->getString("username");
            member["account"] = res->getString("account");
            member["role"] = res->getString("role");
            members.push_back(member);
        }

        response["status"] = "success";
        response["group_name"] = group_name;
        response["members"] = members;
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 创建群
void create_group_msg(int fd, const json& request) {
    json response;
    response["type"] = "create_group";

    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string user_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    if (group_name.empty()) {
        response["status"] = "fail";
        response["msg"] = "Group name cannot be empty";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 检查群名是否已存在
        auto check_stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT 1 FROM chat_groups WHERE group_name = ? LIMIT 1"));
        check_stmt->setString(1, group_name);
        auto check_res = std::unique_ptr<sql::ResultSet>(check_stmt->executeQuery());

        if (check_res->next()) {
            response["status"] = "fail";
            response["msg"] = "Group name already exists";
            send_json(fd, response);
            return;
        }

        // 插入群表
        auto insert_group = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("INSERT INTO chat_groups (group_name, owner_account) VALUES (?, ?)"));
        insert_group->setString(1, group_name);
        insert_group->setString(2, user_account);
        insert_group->executeUpdate();

        // 获取新群 ID
        auto id_stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT LAST_INSERT_ID() AS group_id"));
        auto id_res = std::unique_ptr<sql::ResultSet>(id_stmt->executeQuery());
        int group_id = -1;
        if (id_res->next()) {
            group_id = id_res->getInt("group_id");
        }

        // 插入成员表，角色为 owner
        auto insert_member = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("INSERT INTO group_members (group_id, account, role) VALUES (?, ?, 'owner')"));
        insert_member->setInt(1, group_id);
        insert_member->setString(2, user_account);
        insert_member->executeUpdate();

        response["status"] = "success";
        response["msg"] = "Group created successfully";
        response["group_id"] = group_id;
        response["group_name"] = group_name;

    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 设置管理员
void set_group_admin_msg(int fd, const json& request) {
    json response;
    response["type"] = "set_group_admin";
    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string target_name = request.value("target_name", "");  // 目标成员名字改名为target_name
    std::string user_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    if (group_name.empty() || target_name.empty()) {
        response["status"] = "fail";
        response["msg"] = "Invalid parameters";// 没有参数
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        int group_id = -1;
        std::string group_owner;

        // 查询群ID和群主账号
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT group_id, owner_account FROM chat_groups WHERE group_name = ?"
            ));
            stmt->setString(1, group_name);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            if (!res->next()) {
                response["status"] = "fail";
                response["msg"] = "Group not found";
                send_json(fd, response);
                return;
            }

            group_id = res->getInt("group_id");
            group_owner = res->getString("owner_account");
        }

        // 判断是否是群主
        if (group_owner != user_account) {
            response["status"] = "fail";
            response["msg"] = "Only the group owner can set admins";
            send_json(fd, response);
            return;
        }

        // 根据目标名字查询账号
        std::string target_account;
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                "FROM users "
                "WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?"
            ));

            stmt->setString(1, target_name);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (!res->next()) {
                response["status"] = "fail";
                response["msg"] = "Target user not found";
                send_json(fd, response);
                return;
            }
            target_account = res->getString("account");
        }

        if(user_account == target_account){
            response["status"] = "fail";
            response["msg"] = "Target user can not be owner";
            send_json(fd, response);
            return;
        }

        // 设置管理员
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "UPDATE group_members SET role = 'admin' WHERE group_id = ? AND account = ?"
            ));
            stmt->setInt(1, group_id);
            stmt->setString(2, target_account);
            int affected = stmt->executeUpdate();

            // 受到影响的函数
            if (affected == 0) {
                response["status"] = "fail";
                response["msg"] = "Target user is not a group member";
                send_json(fd, response);
                return;
            }
        }

        response["status"] = "success";
        response["msg"] = "Admin role assigned";

    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 移除管理员
void remove_group_admin_msg(int fd, const json& request) {
    json response;
    response["type"] = "remove_group_admin";
    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string target_name = request.value("target_name", "");
    std::string user_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }
    if (group_name.empty() || target_name.empty()) {
        response["status"] = "fail";
        response["msg"] = "Invalid parameters";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        int group_id = -1;
        std::string group_owner;

        // 查询群ID和群主账号
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT group_id, owner_account FROM chat_groups WHERE group_name = ?"
            ));
            stmt->setString(1, group_name);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (!res->next()) {
                response["status"] = "fail";
                response["msg"] = "Group not found";
                send_json(fd, response);
                return;
            }
            group_id = res->getInt("group_id");
            group_owner = res->getString("owner_account");
        }

        // 判断是否是群主
        if (group_owner != user_account) {
            response["status"] = "fail";
            response["msg"] = "Only the group owner can remove admins";
            send_json(fd, response);
            return;
        }

        // 根据目标名字查询账号
        std::string target_account;
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?"
            ));

            stmt->setString(1, target_name);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (!res->next()) {
                response["status"] = "fail";
                response["msg"] = "Target user not found";
                send_json(fd, response);
                return;
            }
            target_account = res->getString("account");
        }

        // 移除管理员身份
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "UPDATE group_members SET role = 'member' WHERE group_id = ? AND account = ?"
            ));
            stmt->setInt(1, group_id);
            stmt->setString(2, target_account);
            int affected = stmt->executeUpdate();

            if (affected == 0) {
                response["status"] = "fail";
                response["msg"] = "Target user is not a member";
                send_json(fd, response);
                return;
            }
        }

        response["status"] = "success";
        response["msg"] = "Removed admin successfully";

    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();

    }
    send_json(fd, response);
}

// 移除群成员
void remove_group_member_msg(int fd, const json& request) {
    json response;
    response["type"] = "remove_group_member";
    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string target_username = request.value("target_name", "");
    std::string operator_account;

    if (!verify_token(token, operator_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }
    if (group_name.empty() || target_username.empty()) {
        response["status"] = "fail";
        response["msg"] = "Invalid parameters";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 先根据群名查 group_id
        int group_id = -1;
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT group_id FROM chat_groups WHERE group_name = ?"
            ));
            stmt->setString(1, group_name);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (res->next()) {
                group_id = res->getInt("group_id");
            } else {
                response["status"] = "fail";
                response["msg"] = "Group not found";
                send_json(fd, response);
                return;
            }
        }

        // 根据用户名查目标账号
        std::string target_account;
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?"
            ));
            stmt->setString(1, target_username);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (res->next()) {
                target_account = res->getString("account");
            } else {
                response["status"] = "fail";
                response["msg"] = "Target user not found";
                send_json(fd, response);
                return;
            }
        }

        // 验证操作者权限（owner 或 admin）
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT role FROM group_members WHERE group_id = ? AND account = ?"
            ));
            stmt->setInt(1, group_id);
            stmt->setString(2, operator_account);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            if (!res->next()) {
                response["status"] = "fail";
                response["msg"] = "Operator is not a group member";
                send_json(fd, response);
                return;
            }
            std::string role = res->getString("role");
            if (role != "owner" && role != "admin") {
                response["status"] = "fail";
                response["msg"] = "Only owner or admin can remove members";
                send_json(fd, response);
                return;
            }
        }

        // 删除目标成员
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "DELETE FROM group_members WHERE group_id = ? AND account = ?"
            ));
            stmt->setInt(1, group_id);
            stmt->setString(2, target_account);
            int affected = stmt->executeUpdate();

            if (affected == 0) {
                response["status"] = "fail";
                response["msg"] = "Target user is not a member";
                send_json(fd, response);
                return;
            }
        }

        response["status"] = "success";
        response["msg"] = "Removed group member successfully";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }
    send_json(fd, response);
}
 
// 拉成员
void add_group_member_msg(int fd, const json& request) {
    json response;
    response["type"] = "add_group_member";

    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string target_username = request.value("new_member", "");  // 对应客户端传的字段
    std::string operator_account;

    if (!verify_token(token, operator_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    if (group_name.empty() || target_username.empty()) {
        response["status"] = "fail";
        response["msg"] = "Invalid parameters";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 根据群名查 group_id 和群主
        int group_id = -1;
        std::string group_owner;
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT group_id, owner_account FROM chat_groups WHERE group_name = ?"
            ));
            stmt->setString(1, group_name);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (res->next()) {
                group_id = res->getInt("group_id");
                group_owner = res->getString("owner_account");
            } else {
                response["status"] = "fail";
                response["msg"] = "Group not found";
                send_json(fd, response);
                return;
            }
        }

        // 判断操作者是否是群主或管理员
        std::string role;
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT role FROM group_members WHERE group_id = ? AND account = ?"
            ));
            stmt->setInt(1, group_id);
            stmt->setString(2, operator_account);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (res->next()) {
                role = res->getString("role");
            } else {
                response["status"] = "fail";
                response["msg"] = "You are not a group member";
                send_json(fd, response);
                return;
            }
        }

        if (role != "owner" && role != "admin") {
            response["status"] = "fail";
            response["msg"] = "Only owner or admin can add members";
            send_json(fd, response);
            return;
        }

        // 根据用户名查目标账号
        std::string target_account;
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) AS account "
                "FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) = ?"
            ));
            stmt->setString(1, target_username);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (res->next()) {
                target_account = res->getString("account");
            } else {
                response["status"] = "fail";
                response["msg"] = "Target user not found";
                send_json(fd, response);
                return;
            }
        }

        // 确认好友关系且判断屏蔽（对方是否屏蔽自己）
        bool is_friend = false;
        bool target_muted_operator = false;
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT friends FROM friends WHERE account = ?"));
            stmt->setString(1, target_account);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                json friends = json::parse(std::string(res->getString("friends")));
                for (const auto& f : friends) {
                    if (f.value("account", "") == operator_account) {
                        is_friend = true;
                        target_muted_operator = f.value("muted", false);
                        break;
                    }
                }
            }
        }

        if(!is_friend){
            response["status"] = "fail";
            response["msg"] = "User is not your friend";
            send_json(fd, response);
            return;
        }

        if(target_muted_operator){
            response["status"] = "fail";
            response["msg"] = "You are muted";
            send_json(fd, response);
            return;
        }

        // 检查是否已经是成员
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT 1 FROM group_members WHERE group_id = ? AND account = ?"
            ));
            stmt->setInt(1, group_id);
            stmt->setString(2, target_account);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (res->next()) {
                response["status"] = "fail";
                response["msg"] = "User is already a group member";
                send_json(fd, response);
                return;
            }
        }

        // 插入成员记录，默认角色 member
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "INSERT INTO group_members (group_id, account, role) VALUES (?, ?, 'member')"
            ));
            stmt->setInt(1, group_id);
            stmt->setString(2, target_account);
            stmt->executeUpdate();
        }

        response["status"] = "success";
        response["msg"] = "User added to group successfully";

    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }

    send_json(fd, response);
}

// 解散群
void dismiss_group_msg(int fd, const json& request) {
    json response;
    response["type"] = "dismiss_group";
    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string user_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 根据群名查 group_id 和群主
        int group_id = -1;
        std::string group_owner;
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT group_id, owner_account FROM chat_groups WHERE group_name = ?"
            ));
            stmt->setString(1, group_name);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (res->next()) {
                group_id = res->getInt("group_id");
                group_owner = res->getString("owner_account");
                if(group_owner != user_account){
                    response["status"] = "fail";
                    response["msg"] = "Only group owner can dismiss group";
                    send_json(fd, response);
                    return;
                }
            } else {
                response["status"] = "fail";
                response["msg"] = "Group not found";
                send_json(fd, response);
                return;
            }
        }

        // 删除群成员
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("DELETE FROM group_members WHERE group_id = ?"));
            stmt->setInt(1, group_id);
            stmt->execute();
        }

        // 删除入群申请记录
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("DELETE FROM group_requests WHERE group_id = ?"));
            stmt->setInt(1, group_id);
            stmt->execute();
        }

        // 删除群消息
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("DELETE FROM group_messages WHERE group_id = ?"));
            stmt->setInt(1, group_id);
            stmt->execute();
        }

        // 删除群本身
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("DELETE FROM chat_groups WHERE group_id = ?"));
            stmt->setInt(1, group_id);
            stmt->execute();
        }

        response["status"] = "success";
        response["msg"] = "Group dismissed successfully";

    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }
    send_json(fd, response);
}

// 获取群申请
void get_group_requests_msg(int fd, const json& request) {
    json response;
    response["type"] = "get_group_requests";
    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string user_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 获取群id
        int group_id;
        {
            auto stmt = conn->prepareStatement("SELECT group_id FROM chat_groups WHERE group_name = ? ");
            stmt->setString(1, group_name);
            auto res = stmt->executeQuery();

            if (!res->next()) {
                response["status"] = "fail";
                response["msg"] = "Group not found";
                send_json(fd, response);
                return;
            }
            group_id = res->getInt("group_id");
        }   

        // 验证操作者权限（owner 或 admin）
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT role FROM group_members WHERE group_id = ? AND account = ?"
            ));
            stmt->setInt(1, group_id);
            stmt->setString(2, user_account);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            if (!res->next()) {
                response["status"] = "fail";
                response["msg"] = "Operator is not a group member";
                send_json(fd, response);
                return;
            }
            std::string role = res->getString("role");
            if (role != "owner" && role != "admin") {
                response["status"] = "fail";
                response["msg"] = "Only owner or admin can get group request";
                send_json(fd, response);
                return;
            }
        }

        auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
            "SELECT sender FROM group_requests WHERE group_id = ?"
        ));
        stmt->setInt(1, group_id);
        auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

        json requests = json::array();
        while (res->next()) {
            std::string sender_account = res->getString("sender");
            std::string sender_username;
            {
                std::unique_ptr<sql::PreparedStatement> uname_stmt(
                    conn->prepareStatement(
                        "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) AS username FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) = ?"));
                uname_stmt->setString(1, sender_account);
                std::unique_ptr<sql::ResultSet> uname_res(uname_stmt->executeQuery());

                if (uname_res->next()) {
                    sender_username = uname_res->getString("username");
                }
            }

            requests.push_back({
                {"account", sender_account},
                {"username", sender_username},
                {"group_name", group_name}
            });
        }

        response["status"] = "success";
        response["msg"] = "Get list success";
        response["requests"] = requests;
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }
    send_json(fd, response);
}

// 处理群申请
void handle_group_request_msg(int fd, const json& request) {
    json response;
    response["type"] = "handle_group_request";
    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string from_username = request.value("from_username", "");
    std::string action = request.value("action", "reject");
    std::string user_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        int group_id;
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT group_id FROM chat_groups WHERE group_name = ? "
            ));
            stmt->setString(1, group_name);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            if (!res->next()) {
                response["status"] = "fail";
                response["msg"] = "Group not found";
                send_json(fd, response);
                return;
            }
            group_id = res->getInt("group_id");
        }

        if (action == "accept") {
            {
                auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                    "INSERT IGNORE INTO group_members (group_id, account, role) VALUES (?, ?, 'member')"
                ));
                stmt->setInt(1, group_id);
                stmt->setString(2, from_username);
                stmt->execute();
            }
        }

        std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
            "DELETE FROM group_requests WHERE group_id = ? AND sender = ?"
        ));
        stmt->setInt(1, group_id);
        stmt->setString(2, from_username);
        stmt->execute();

        response["status"] = "success";
        response["msg"] = (action == "accept") ? "Request accepted" : "Request rejected";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }
    send_json(fd, response);
}







































// 获取未读消息
void get_unread_group_messages_msg(int fd, const json& request) {
    json response;
    response["type"] = "get_unread_group_messages";
    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string user_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();
        int group_id = -1;
        int last_read_id = 0;

        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT group_id FROM chat_groups WHERE group_name = ?"));
            stmt->setString(1, group_name);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (!res->next()) {
                response["status"] = "fail";
                response["msg"] = "Group not found";
                send_json(fd, response);
                return;
            }
            group_id = res->getInt("group_id");
        }

        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT last_read_message_id FROM group_read_status WHERE group_id = ? AND account = ?"));
            stmt->setInt(1, group_id);
            stmt->setString(2, user_account);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (res->next()) {
                last_read_id = res->getInt("last_read_message_id");
            }
        }

        json messages = json::array();
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT id, sender, content, timestamp FROM group_messages WHERE group_id = ? AND id > ? ORDER BY id ASC"));
            stmt->setInt(1, group_id);
            stmt->setInt(2, last_read_id);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            int max_id = last_read_id;
            while (res->next()) {
                int msg_id = res->getInt("id");
                if (msg_id > max_id) max_id = msg_id;
                messages.push_back({
                    {"sender", res->getString("sender")},
                    {"content", res->getString("content")},
                    {"timestamp", res->getString("timestamp")}
                });
            }

            if (!messages.empty()) {
                std::unique_ptr<sql::PreparedStatement> update_stmt(
                    conn->prepareStatement(
                "INSERT INTO group_read_status (group_id, account, last_read_message_id) "
                "VALUES (?, ?, ?) "
                "ON DUPLICATE KEY UPDATE last_read_message_id = VALUES(last_read_message_id)"
                ));

                update_stmt->setInt(1, group_id);
                update_stmt->setString(2, user_account);
                update_stmt->setInt(3, max_id);
                update_stmt->execute();
            }
        }

        response["status"] = "success";
        response["messages"] = messages;
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }
    send_json(fd, response);
}


// 获取群历史信息
void get_group_history_msg(int fd, const json& request) {
    json response;
    response["type"] = "get_group_history";
    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    int count = request.value("count", 10);  // 默认返回10条
    std::string user_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();
        int group_id = -1;

        // 获取群id
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT group_id FROM chat_groups WHERE group_name = ?"));
            stmt->setString(1, group_name);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (!res->next()) {
                response["status"] = "fail";
                response["msg"] = "Group not found";
                send_json(fd, response);
                return;
            }
            group_id = res->getInt("group_id");
        }

        // 查询最近 count 条历史消息
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "SELECT sender, content, timestamp FROM group_messages "
                "WHERE group_id = ? "
                "ORDER BY timestamp DESC LIMIT ?"
            )
        );
        stmt->setInt(1, group_id);
        stmt->setInt(2, count);
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        json history = json::array();
        while (res->next()) {
            history.push_back({
                {"sender", res->getString("sender")},
                {"content", res->getString("content")},
                {"timestamp", res->getString("timestamp")}
            });
        }

        // 倒序改正为正序（按时间正向显示）
        std::reverse(history.begin(), history.end());

        response["status"] = "success";
        response["messages"] = history;
        response["group_name"] = group_name;
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }
    send_json(fd, response);
}

// 发送群聊消息
void send_group_message_msg(int fd, const json& request) {
    json response,push_msg;
    response["type"] = "send_group_message";
    push_msg["type"] = "receive_group_messages";

    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string message = request.value("message", "");
    std::string user_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 获取群id
        int group_id = -1;
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT group_id FROM chat_groups WHERE group_name = ?"));
            stmt->setString(1, group_name);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            if (!res->next()) {
                response["status"] = "fail";
                response["msg"] = "Group not found";
                send_json(fd, response);
                return;
            }
            group_id = res->getInt("group_id");
        }

        // 判断是否是群成员
        bool is_member = false;
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT 1 FROM group_members WHERE group_id = ? AND account = ?"));
            stmt->setInt(1, group_id);
            stmt->setString(2, user_account);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            is_member = res->next();
        }

        if (!is_member) {
            response["status"] = "fail";
            response["msg"] = "You are not a member of this group";
            send_json(fd, response);
            return;
        }

        // 插入消息到 group_messages 表
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "INSERT INTO group_messages (group_id, sender, content) VALUES (?, ?, ?)"));
            stmt->setInt(1, group_id);
            stmt->setString(2, user_account);
            stmt->setString(3, message);
            stmt->executeUpdate();
        }

        // 准备推送内容
        push_msg["group_name"] = group_name;
        push_msg["from"] = user_account;
        push_msg["message"] = message;

        // 推送给所有在线群成员（除了自己）
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT account FROM group_members WHERE group_id = ?"));
            stmt->setInt(1, group_id);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

            while (res->next()) {
                std::string member = res->getString("account");
                if (member == user_account) continue;
                if (redis.exists("online:" + member)) {
                    int member_fd = get_fd_by_account(member);
                    if (member_fd != -1) {
                        send_json(member_fd, push_msg);
                    }
                }
            }
        }

        response["status"] = "success";
        response["msg"] = "Message sent";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }
    send_json(fd, response);
}




























































































void error_msg(int fd, const nlohmann::json &request){
    json response;
    response["type"] = "error";
    response["msg"] = "Unrecognized request type";
    send_json(fd, response);
}


