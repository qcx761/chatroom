

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













// 群
// CREATE TABLE groups (
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
//     FOREIGN KEY (group_id) REFERENCES groups(group_id) ON DELETE CASCADE
// );

// 成员在群信息
// CREATE TABLE group_members (
//     group_id INT NOT NULL COMMENT,                                     -- 群聊 ID，关联 groups 表的主键
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
















void join_group_msg(int fd, const json& request) {
    json response;
    response["type"] = "join_group";

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
        std::unique_ptr<sql::PreparedStatement> stmt;
        std::unique_ptr<sql::ResultSet> res;

        // 查 group_id
        int group_id = -1;
        stmt = conn->prepareStatement("SELECT group_id FROM groups WHERE group_name = ?");
        stmt->setString(1, group_name);
        res = stmt->executeQuery();
        if (res->next()) {
            group_id = res->getInt("group_id");
        } else {
            response["status"] = "fail";
            response["msg"] = "Group not found";
            send_json(fd, response);
            return;
        }

        // 检查是否已经是群成员
        stmt = conn->prepareStatement(
            "SELECT role FROM group_members WHERE group_id = ? AND account = ?");
        stmt->setInt(1, group_id);
        stmt->setString(2, user_account);
        res = stmt->executeQuery();
        if (res->next()) {
            response["status"] = "fail";
            response["msg"] = "Already a group member";
            send_json(fd, response);
            return;
        }

        // 检查是否已有申请记录
        stmt = conn->prepareStatement(
            "SELECT id, status FROM group_requests WHERE sender = ? AND group_id = ? ORDER BY id DESC");
        stmt->setString(1, user_account);
        stmt->setInt(2, group_id);
        res = stmt->executeQuery();

        if (res->next()) {
            int latest_id = res->getInt("id");
            std::string last_status = res->getString("status");

            if (last_status == "pending") {
                // 刷新申请时间
                stmt = conn->prepareStatement("UPDATE group_requests SET timestamp = NOW() WHERE id = ?");
                stmt->setInt(1, latest_id);
                stmt->execute();

                response["status"] = "success";
                response["msg"] = "Join request refreshed";
                send_json(fd, response);
                return;
            } else if (last_status == "accepted") {
                response["status"] = "fail";
                response["msg"] = "Already a group member";
                send_json(fd, response);
                return;
            }

            // 否则重新发送申请
            stmt = conn->prepareStatement(
                "UPDATE group_requests SET status = 'pending', timestamp = NOW() WHERE id = ?");
            stmt->setInt(1, latest_id);
            stmt->execute();

            response["status"] = "success";
            response["msg"] = "Join request re-sent";
            send_json(fd, response);
            return;
        }

        // 插入新申请
        stmt = conn->prepareStatement(
            "INSERT INTO group_requests(sender, group_id, status, timestamp) VALUES (?, ?, 'pending', NOW())");
        stmt->setString(1, user_account);
        stmt->setInt(2, group_id);
        stmt->execute();

        response["status"] = "success";
        response["msg"] = "Join request sent";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}

void join_group_msg(int fd, const json& request) {
    json response;
    response["type"] = "join_group";

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

        // 查询group_id
        int group_id = -1;
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT group_id FROM groups WHERE group_name = ?"
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

        // 检查用户是否已经是群成员
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "SELECT * FROM group_members WHERE group_id = ? AND user_account = ?"
            ));
            stmt->setInt(1, group_id);
            stmt->setString(2, user_account);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());

            if (res->next()) {
                response["status"] = "fail";
                response["msg"] = "You are already in the group";
                send_json(fd, response);
                return;
            }
        }

        // 插入群成员表
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(conn->prepareStatement(
                "INSERT INTO group_members(group_id, user_account, is_admin) VALUES (?, ?, 0)"
            ));
            stmt->setInt(1, group_id);
            stmt->setString(2, user_account);
            stmt->execute();
        }

        response["status"] = "success";
        response["msg"] = "Joined group successfully";

    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}
