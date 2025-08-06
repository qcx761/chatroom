

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






















































