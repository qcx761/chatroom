

void show_group_list_msg(int fd, const json& request){
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

    try
    {
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









        
    }
    catch(const std::exception& e)
    {
        response["status"] = "error";
        response["msg"] = e.what();
    }
    


}

void join_group_msg(int fd, const json& request){
    ;
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



// 1. 删除 users 表记录
        {
            auto del_users = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("DELETE FROM users WHERE id = ?"));
            del_users->setInt(1, id);
            del_users->executeUpdate();
        }

        // 2. 删除 friends 表中本人好友列表
        {
            auto del_friends = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("DELETE FROM friends WHERE account = ?"));
            del_friends->setString(1, account);
            del_friends->executeUpdate();
        }

        // 3. 从其他用户的好友列表中移除自己
        {
            auto get_all_friends = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("SELECT account, friends FROM friends"));
            auto all_res = std::unique_ptr<sql::ResultSet>(get_all_friends->executeQuery());

            while (all_res->next()) {
                std::string acc = all_res->getString("account");
                json friends = json::parse(all_res->getString("friends"));
                json new_friends = json::array();
                bool changed = false;

                for (const auto &f : friends) {
                    if (f.value("account", "") != account) {
                        new_friends.push_back(f);
                    } else {
                        changed = true;
                    }
                }

                if (changed) {
                    auto update_stmt = std::unique_ptr<sql::PreparedStatement>(
                        conn->prepareStatement("REPLACE INTO friends(account, friends) VALUES (?, ?)"));
                    update_stmt->setString(1, acc);
                    update_stmt->setString(2, new_friends.dump());
                    update_stmt->execute();
                }
            }
        }

        // 4. 删除好友请求
        {
            auto del_requests = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("DELETE FROM friend_requests WHERE sender = ? OR receiver = ?"));
            del_requests->setString(1, account);
            del_requests->setString(2, account);
            del_requests->executeUpdate();
        }

        // 5. 删除私聊消息
        {
            auto del_messages = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("DELETE FROM messages WHERE sender = ? OR receiver = ?"));
            del_messages->setString(1, account);
            del_messages->setString(2, account);
            del_messages->executeUpdate();
        }

        // 6. 删除本人创建的群聊（chat_groups 级联）
        {
            auto del_groups = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("DELETE FROM chat_groups WHERE owner_account = ?"));
            del_groups->setString(1, account);
            del_groups->executeUpdate();
        }

        // 7. 删除自己在群成员表中的记录
        {
            auto del_group_members = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("DELETE FROM group_members WHERE account = ?"));
            del_group_members->setString(1, account);
            del_group_members->executeUpdate();
        }

        // 8. 删除自己发出的加群请求
        {
            auto del_group_requests = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("DELETE FROM group_requests WHERE sender = ?"));
            del_group_requests->setString(1, account);
            del_group_requests->executeUpdate();
        }

        // 9. 删除群聊消息（如果没有 ON DELETE CASCADE）
        {
            auto del_group_msgs = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("DELETE FROM group_messages WHERE sender = ?"));
            del_group_msgs->setString(1, account);
            del_group_msgs->executeUpdate();
        }

        // 10. 删除文件消息
        {
            auto del_file_msgs = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("DELETE FROM file_messages WHERE sender = ? OR receiver = ?"));
            del_file_msgs->setString(1, account);
            del_file_msgs->setString(2, account);
            del_file_msgs->executeUpdate();

            auto del_group_file_msgs = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("DELETE FROM group_file_messages WHERE sender = ?"));
            del_group_file_msgs->setString(1, account);
            del_group_file_msgs->executeUpdate();
        }

        // 11. 删除群聊阅读状态
        {
            auto del_group_read_status = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("DELETE FROM group_read_status WHERE account = ?"));
            del_group_read_status->setString(1, account);
            del_group_read_status->executeUpdate();
        }





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
//     id INT AUTO_INCREMENT PRIMARY KEY,      -- 消息唯一ID，自增主键
//     sender VARCHAR(64),                     -- 发送者账号
//     receiver VARCHAR(64),                   -- 接收者账号
//     content TEXT,                           -- 消息内容，文本类型
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,  -- 消息发送时间，默认当前时间
//     is_online BOOLEAN DEFAULT FALSE,       -- 发送时接收者是否在线，默认为否
//     is_read BOOLEAN DEFAULT FALSE          -- 消息是否已读标志，默认为否
// );




// 群
// CREATE TABLE chat_groups (
//     group_id INT PRIMARY KEY AUTO_INCREMENT,        -- 群ID，自增
//     group_name VARCHAR(64) NOT NULL UNIQUE,         -- 群聊名，唯一
//     owner_account VARCHAR(64) NOT NULL,             -- 群主账号
// );

// 群消息
// CREATE TABLE group_messages (
//     id INT PRIMARY KEY AUTO_INCREMENT,               -- 消息ID
//     group_id INT NOT NULL,                           -- 所属群ID
//     sender VARCHAR(64) NOT NULL,                     -- 发送者账号
//     content TEXT NOT NULL,                           -- 消息内容
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,    -- 发送时间
// );

// 成员在群信息
// CREATE TABLE group_members (
//     group_id INT NOT NULL COMMENT,                                     -- 群聊 ID，关联 chat_groups 表的主键
//     account VARCHAR(64) NOT NULL COMMENT,                              -- 用户账号，关联 users 表的主键或唯一字段
//     role ENUM('owner', 'admin', 'member') DEFAULT 'member' COMMENT,    -- 在群中的角色：群主、管理员或普通成员
// );

// CREATE TABLE group_requests (
//     id INT PRIMARY KEY AUTO_INCREMENT,                         -- 主键ID，自动递增
//     sender VARCHAR(64) NOT NULL,                               -- 发起申请的用户账号
//     group_id INT NOT NULL,                                     -- 要加入的群聊 ID
//     status ENUM('pending', 'accepted', 'rejected') NOT NULL    -- 当前状态（待处理 / 接受 / 拒绝）
//         DEFAULT 'pending',
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,              -- 申请发起时间
// );




// CREATE TABLE group_read_status (
//     group_id INT NOT NULL,
//     account VARCHAR(64) NOT NULL,
//     last_read_message_id INT DEFAULT 0,
//     PRIMARY KEY (group_id, account)
// );


// 群
// CREATE TABLE chat_groups (
//     group_id INT PRIMARY KEY AUTO_INCREMENT,        
//     group_name VARCHAR(64) NOT NULL UNIQUE,         
//     owner_account VARCHAR(64) NOT NULL            
// );

// 群消息
// CREATE TABLE group_messages (
//     id INT PRIMARY KEY AUTO_INCREMENT,               
//     group_id INT NOT NULL,                           
//     sender VARCHAR(64) NOT NULL,                     
//     content TEXT NOT NULL,                           
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
// );


 
// 成员在群信息
// CREATE TABLE group_members (
//     group_id INT NOT NULL,                                     
//     account VARCHAR(64) NOT NULL,                              
//     role ENUM('owner', 'admin', 'member') DEFAULT 'member' 
// );

// CREATE TABLE group_requests (
//     id INT PRIMARY KEY AUTO_INCREMENT,                         
//     sender VARCHAR(64) NOT NULL,                               
//     group_id INT NOT NULL,                                     
//     status ENUM('pending', 'accepted', 'rejected') NOT NULL    
//         DEFAULT 'pending',
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP              
// );
md5sum



/home/kong/test/ceshi/ceshi.cpp

/home/kong/test/1GB_testfile.bin




CREATE TABLE file_messages (
    id INT AUTO_INCREMENT PRIMARY KEY,
    sender VARCHAR(64) NOT NULL,
    receiver VARCHAR(64) NOT NULL,
    filename VARCHAR(255) NOT NULL,
    filesize VARCHAR(64),
    filepath VARCHAR(1024) NOT NULL,
    is_read BOOLEAN DEFAULT FALSE,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE group_file_messages (
    id INT AUTO_INCREMENT PRIMARY KEY,
    sender VARCHAR(64) NOT NULL,
    group_id INT NOT NULL,
    filename VARCHAR(255) NOT NULL,
    filesize VARCHAR(64),
    filepath VARCHAR(1024) NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);






void handle_get_file_list(int fd, const json& request) {
    json response;
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
        json file_list = json::array();

        // 1️⃣ 私聊文件
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "SELECT id, sender, filename, filesize, filepath, timestamp "
                    "FROM file_messages WHERE receiver = ? ORDER BY timestamp DESC"
                )
            );
            stmt->setString(1, user_account);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

            while (res->next()) {
                json file;
                file["type"] = "private";
                file["id"] = res->getInt("id");
                file["sender"] = res->getString("sender");
                file["filename"] = res->getString("filename");
                file["filesize"] = res->getString("filesize");
                file["filepath"] = res->getString("filepath");
                file["timestamp"] = res->getString("timestamp");
                file_list.push_back(file);
            }
        }

        // 2️⃣ 群聊文件：查询自己参与的群
        std::set<int> group_ids;
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT group_id FROM group_members WHERE account = ?")
            );
            stmt->setString(1, user_account);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
            while (res->next()) {
                group_ids.insert(res->getInt("group_id"));
            }
        }

        // 查询这些群的群文件
        if (!group_ids.empty()) {
            std::string in_clause;
            for (int gid : group_ids) {
                in_clause += std::to_string(gid) + ",";
            }
            in_clause.pop_back();  // 去掉最后的逗号

            std::string sql = 
                "SELECT id, group_id, sender, filename, filesize, filepath, timestamp "
                "FROM group_file_messages WHERE group_id IN (" + in_clause + ") ORDER BY timestamp DESC";

            std::unique_ptr<sql::Statement> stmt(conn->createStatement());
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(sql));

            while (res->next()) {
                json file;
                file["type"] = "group";
                file["id"] = res->getInt("id");
                file["group_id"] = res->getInt("group_id");
                file["sender"] = res->getString("sender");
                file["filename"] = res->getString("filename");
                file["filesize"] = res->getString("filesize");
                file["filepath"] = res->getString("filepath");
                file["timestamp"] = res->getString("timestamp");
                file_list.push_back(file);
            }
        }

        response["status"] = "ok";
        response["files"] = file_list;
        send_json(fd, response);
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
        send_json(fd, response);
    }
}



















std::string filepath = "/home/kong/plan/chartroom/chat-system/server/files/" + user_account + "/" + filename;



void send_private_file_msg(int fd, const json& request){
    json response;

    std::string token = request.value("token", "");
    std::string target_username = request.value("target_name", "");
    std::string filename = request.value("filename", "");
    std::string filesize = request.value("filesize", "");
    std::string user_account;
    std::string target_account;

    if (!verify_token(token, user_account)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

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

        // 假设你已知服务器保存文件路径，拼成filepath
        // std::string filepath = "/var/ftp/files/" + user_account + "/" + filename;
        std::string filepath = "/home/kong/plan/chartroom/chat-system/server/"  + filename;


        // 判断对方是否在线
        bool is_online = redis.exists("online:" + target_account);

        // 存储文件消息记录
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "INSERT INTO file_messages (sender, receiver, filename, filesize, filepath, is_online, is_read, timestamp) "
                    "VALUES (?, ?, ?, ?, ?, ?, FALSE, NOW())"));
            stmt->setString(1, user_account);
            stmt->setString(2, target_account);
            stmt->setString(3, filename);
            stmt->setString(4, filesize);
            stmt->setString(5, filepath);
            stmt->setBoolean(6, is_online);
            stmt->execute();
        }

        // 在线则推送通知
        if (is_online) {
            int target_fd = get_fd_by_account(target_account);
            if (target_fd == -1) {
                response["status"] = "fail";
                response["msg"] = "Failed to get target user fd";
                send_json(fd, response);
                return;
            }

            json push_msg;
            push_msg["type"] = "file_receive";
            push_msg["from"] = user_account;
            push_msg["filename"] = filename;
            push_msg["filesize"] = filesize;
            push_msg["filepath"] = filepath;
            send_json(target_fd, push_msg);
        }

        response["status"] = "success";
        send_json(fd, response);
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
        send_json(fd, response);
    }
}





void send_group_file_msg(int fd, const json& request){
    json response;

    std::string token = request.value("token", "");
    std::string group_name = request.value("group_name", "");
    std::string filename = request.value("filename", "");
    std::string filesize = request.value("filesize", "");
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
        // 查找群id
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

        // 假设服务器文件保存路径
        // std::string filepath = "/var/ftp/files/groups/" + std::to_string(group_id) + "/" + filename;
        std::string filepath = "/home/kong/plan/chartroom/chat-system/server/"  + filename;

        // 存储群文件消息记录
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement(
                    "INSERT INTO group_file_messages (sender, group_id, filename, filesize, filepath, timestamp) "
                    "VALUES (?, ?, ?, ?, ?, NOW())"));
            stmt->setString(1, user_account);
            stmt->setInt(2, group_id);
            stmt->setString(3, filename);
            stmt->setString(4, filesize);
            stmt->setString(5, filepath);
            stmt->execute();
        }

        // 推送给在线群成员（除自己外）
        {
            std::unique_ptr<sql::PreparedStatement> stmt(
                conn->prepareStatement("SELECT account FROM group_members WHERE group_id = ?"));
            stmt->setInt(1, group_id);
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

            while (res->next()) {
                std::string member_account = res->getString("account");
                if (member_account == user_account) continue;
                if (redis.exists("online:" + member_account)) {
                    int member_fd = get_fd_by_account(member_account);
                    if (member_fd != -1) {
                        json push_msg;
                        push_msg["type"] = "group_file_receive";
                        push_msg["from"] = user_account;
                        push_msg["group_name"] = group_name;
                        push_msg["filename"] = filename;
                        push_msg["filesize"] = filesize;
                        push_msg["filepath"] = filepath;
                        send_json(member_fd, push_msg);
                    }
                }
            }
        }

        response["status"] = "success";
        send_json(fd, response);

    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
        send_json(fd, response);
    }
}




/home/kong/test/ceshi/5g_file.bin









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



-- 文件传输元信息表
CREATE TABLE file_transfers (
    file_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    sender VARCHAR(64) NOT NULL,
    receiver VARCHAR(64) NOT NULL,
    filename VARCHAR(255) NOT NULL,
    filesize BIGINT NOT NULL,
    filepath VARCHAR(512) NOT NULL, -- 服务端实际存储路径
    status ENUM('pending', 'transferring', 'complete', 'cancelled') DEFAULT 'pending',
    upload_offset BIGINT DEFAULT 0,
    download_offset BIGINT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- 离线文件通知表（用于记录接收者未领取的文件）
CREATE TABLE offline_file_notifications (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    file_id BIGINT NOT NULL,
    receiver VARCHAR(64) NOT NULL,
    notified BOOLEAN DEFAULT FALSE,
    received BOOLEAN DEFAULT FALSE,
    FOREIGN KEY (file_id) REFERENCES file_transfers(file_id)
);



























        bool is_online = redis.exists("online:" + target_account);


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












        bool is_online = redis.exists("online:" + target_account);

        if (is_online) {
            int target_fd = get_fd_by_account(target_account);
            if (target_fd == -1) {
                return;
            }

            json resp;
            resp["type"] = "receive_message";
            resp["type1"] = ;
            resp["state"] = ;

            send_json(target_fd, resp);
        }else{
                    ;
//离线

        }





        bool is_online = redis.exists("online:" + target_account);

//         // 在线则推送消息
        if (is_online) {
            int target_fd = get_fd_by_account(target_account);
                json response;
                response["type"] = "receive_message";
                response["type1"] = "private_file_message";

            if (target_fd == -1) {
                response["status"] = "fail";
                response["msg"] = "Failed to get target user fd";
                send_json(fd, response);
                return;
            }

            response["from"] = user_name;
//             response1["message"] = message;


            send_json(target_fd, response);
        }else{
                    ;
//离线
// 上线在哪里调用通知函数
// 离线要怎么实现用户上线提示和发送信息
// 上线的离线消息发送逻辑，遍历消息表输出未读消息
        }


}
















void FTPServer::handle_stor_data(int data_fd, int control_fd) {
    auto it = stor_states.find(data_fd);
    if (it == stor_states.end()) {
        cerr << "No stor state for data_fd: " << data_fd << endl;
        close_connection(data_fd);
        return;
    }

    static size_t total_received = 0;  // 统计本次上传已接收的字节数

    char buf[4096];
    while (true) {
        ssize_t n = recv(data_fd, buf, sizeof(buf), 0);

        if (n > 0) {
            total_received += n;
            cout << "\r上传进度: " << total_received / 1024 << " KB   " << flush;
            cout << "\r上传进度: " << total_received / (1024*1024) << " MB   " << flush;

            // 处理 write() 可能部分写入的情况
            ssize_t total_written = 0;
            while (total_written < n) {
                ssize_t written = write(it->second.file_fd, buf + total_written, n - total_written);
                if (written < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                    perror("write error");
                    close(it->second.file_fd);
                    stor_states.erase(it);
                    close_connection(data_fd);
                    return;
                }
                total_written += written;
            }

        } else if (n == 0) {
            // 客户端关闭了数据连接（传输完成）
            std::cout << "[服务端] 客户端关闭数据连接，发送 226" << std::endl;

            close(it->second.file_fd);
            stor_states.erase(it);
            close_connection(data_fd);

            const char* msg = "226 Transfer complete.\r\n";
            send(control_fd, msg, strlen(msg), 0);
            total_received = 0;  // 重置统计数

            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 没有更多数据了，等下一次 epoll 再进来
                break;
            } else {
                // 发生错误
                perror("recv error");
                close(it->second.file_fd);
                stor_states.erase(it);
                close_connection(data_fd);
                total_received = 0;
                return;
            }
        }
    }
}














CREATE TABLE offline_message_counter (
    id INT AUTO_INCREMENT PRIMARY KEY,
    account VARCHAR(255) NOT NULL,          -- 接收者账号
    sender VARCHAR(255) NOT NULL,           -- 发送者账号或群名
    message_type VARCHAR(50) NOT NULL,      -- private_text / private_file / group_text / group_file
    count INT DEFAULT 0,                    -- 离线消息数量
    UNIQUE KEY (account, sender, message_type)
);



    // 离线处理逻辑：更新计数表
    try {
        auto conn = get_mysql_connection();
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "INSERT INTO offline_message_counter(account, sender, message_type, count) "
                "VALUES (?, ?, ?, 1) "
                "ON DUPLICATE KEY UPDATE count = count + 1"
            ));
        stmt->setString(1, target_account);              // 接收者
        stmt->setString(2, user_account);                // 发送者
        stmt->setString(3, "private_text");              // 消息类型（按你的业务逻辑调整）
        stmt->execute();
    } catch (const std::exception& e) {
        std::cerr << "[offline_message_counter] Error: " << e.what() << std::endl;
    }

    send_offline_summary_on_login(account, fd);


void send_offline_summary_on_login(const std::string& account, int fd) {
    try {
        auto conn = get_mysql_connection();

        // 1. 查询所有未读消息摘要
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "SELECT sender, message_type, count "
                "FROM offline_message_counter WHERE account = ?"));
        stmt->setString(1, account);
        auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

        json summary;
        summary["type"] = "offline_summary";
        summary["status"] = "success";
        json messages = json::array();

        while (res->next()) {
            json entry;
            entry["sender"] = res->getString("sender");
            entry["message_type"] = res->getString("message_type");
            entry["count"] = res->getInt("count");
            messages.push_back(entry);
        }

        summary["messages"] = messages;

        // 2. 向客户端发送摘要
        send_json(fd, summary);

        // 3. 自动清零（删除对应统计）
        std::unique_ptr<sql::PreparedStatement> clear_stmt(
            conn->prepareStatement(
                "DELETE FROM offline_message_counter WHERE account = ?"));
        clear_stmt->setString(1, account);
        clear_stmt->execute();

    } catch (const std::exception& e) {
        std::cerr << "[send_offline_summary_on_login] Error: " << e.what() << std::endl;
    }
}











void FTPServer::handle_stor_data(int data_fd, int control_fd) {
    auto it = stor_states.find(data_fd);
    if (it == stor_states.end()) {
        cerr << "No stor state for data_fd: " << data_fd << endl;
        close_connection(data_fd);
        return;
    }

    static size_t total_received = 0;  // 统计本次上传已接收的字节数

    char buf[4096];
    while (true) {
        ssize_t n = recv(data_fd, buf, sizeof(buf), 0);
        if (n > 0) {
            total_received += n;
            cout << "\r上传进度: " << total_received / 1024 << " KB   " << flush;

            ssize_t written = write(it->second.file_fd, buf, n);
            if (written != n) {
                perror("write error");
                close(it->second.file_fd);
                stor_states.erase(it);
                close_connection(data_fd);
                return;
            }
        } else if (n == 0) {
            cout << endl << "[服务端] 客户端关闭数据连接，发送 226" << endl;
            close(it->second.file_fd);
            stor_states.erase(it);
            close_connection(data_fd);
            const char* msg = "226 Transfer complete.\r\n";
            send(control_fd, msg, strlen(msg), 0);
            total_received = 0;  // 重置统计数
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                perror("recv error");
                close(it->second.file_fd);
                stor_states.erase(it);
                close_connection(data_fd);
                total_received = 0;
                return;
            }
        }
    }
}
