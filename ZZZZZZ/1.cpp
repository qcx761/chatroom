// 已添加的头文件、省略部分重复代码、JSON、Redis、MySQL连接工具等已存在

// ========== 显示好友列表（含在线状态 + 是否屏蔽） ==========
void show_friend_list_msg(int fd, const json& request) {
    json response;
    response["type"] = "show_friend_list";
    std::string token = request.value("token", "");
    std::string account;

    if (!verify_token(token, account)) {
        response["status"] = "fail";
        response["msg"] = "无效 token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();
        auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
        stmt->setString(1, account);
        auto res = stmt->executeQuery();

        std::vector<json> friend_list;
        if (res->next()) {
            json friends = json::parse(res->getString("friends"));
            Redis redis("tcp://127.0.0.1:6379");

            for (const auto& f : friends) {
                std::string friend_account = f.value("account", "");
                std::string friend_username = f.value("username", "");
                bool is_online = redis.exists("online:" + friend_account);

                friend_list.push_back({
                    {"account", friend_account},
                    {"username", friend_username},
                    {"online", is_online},
                    {"muted", f.value("muted", false)}
                });
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

// ========== 屏蔽/解除屏蔽好友 ==========
void is_mute_friend_msg(int fd, const json& request) {
    json response;
    response["type"] = "mute_friend";
    std::string token = request.value("token", "");
    std::string target_account = request.value("target_account", "");
    bool muted = request.value("muted", true);
    std::string user;

    if (!verify_token(token, user)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();
        auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
        stmt->setString(1, user);
        auto res = stmt->executeQuery();

        json friends = json::array();
        if (res->next()) friends = json::parse(res->getString("friends"));

        for (auto& f : friends) {
            if (f["account"] == target_account) {
                f["muted"] = muted;
            }
        }

        auto update = conn->prepareStatement("REPLACE INTO friends(account, friends) VALUES (?, ?)");
        update->setString(1, user);
        update->setString(2, friends.dump());
        update->execute();

        response["status"] = "success";
        response["msg"] = muted ? "好友已屏蔽" : "已取消屏蔽";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }
    send_json(fd, response);
}

{
  "type": "mute_friend",
  "token": "xxx",
  "target_account": "friend001",
  "action": "mute"   // 或者 "unmute"
}




// 添加好友
void add_friend_msg(int fd, const json& request) {
    json response;
    response["type"] = "add_friend";

    std::string token = request.value("token", "");
    std::string sender;

    // 要添加的好友账号
    std::string target_account = request.value("target_account", "");

    if (!verify_token(token, sender)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    if (sender == target_account) {
        response["status"] = "fail";
        response["msg"] = "Cannot add yourself";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        // 检查对方是否存在
        auto check_stmt = conn->prepareStatement("SELECT info FROM users WHERE JSON_EXTRACT(info, '$.account') = ?");
        check_stmt->setString(1, target_account);
        auto check_res = check_stmt->executeQuery();

        if (!check_res->next()) {
            response["status"] = "fail";
            response["msg"] = "Target user does not exist";
            send_json(fd, response);
            return;
        }

        // 检查是否已是好友
        auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
        stmt->setString(1, sender);
        auto res = stmt->executeQuery();

        if (res->next()) {
            json friends = json::parse(res->getString("friends"));
            for (const auto& f : friends) {
                if (f.value("account", "") == target_account) {
                    response["status"] = "fail";
                    response["msg"] = "Already friends";
                    send_json(fd, response);
                    return;
                }
            }
        }

        // 检查是否已发起请求
        auto check_req = conn->prepareStatement(
            "SELECT status FROM friend_requests WHERE sender = ? AND receiver = ?");
        check_req->setString(1, sender);
        check_req->setString(2, target_account);
        auto req_res = check_req->executeQuery();

        if (req_res->next()) {
            std::string status = req_res->getString("status");
            if (status == "pending") {
                response["status"] = "fail";
                response["msg"] = "Request already sent";
                send_json(fd, response);
                return;
            }
        }

        // 插入请求
        auto insert = conn->prepareStatement(
            "INSERT INTO friend_requests(sender, receiver, status, timestamp) VALUES (?, ?, 'pending', NOW())");
        insert->setString(1, sender);
        insert->setString(2, target_account);
        insert->execute();

        // TODO: 可选 - 发推送通知给 target_account，走消息通道/事件队列

        response["status"] = "success";
        response["msg"] = "Friend request sent";
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
    std::string target_account = request.value("target_account", "");
    std::string user;

    // 1. 验证 Token
    if (!verify_token(token, user)) {
        response["status"] = "fail";
        response["msg"] = "Invalid token";
        send_json(fd, response);
        return;
    }

    if (user == target_account) {
        response["status"] = "fail";
        response["msg"] = "Cannot remove yourself";
        send_json(fd, response);
        return;
    }

    try {
        auto conn = get_mysql_connection();

        auto remove_from_friend_list = [&](const std::string& owner, const std::string& remove_account) {
            auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
            stmt->setString(1, owner);
            auto res = stmt->executeQuery();

            if (res->next()) {
                json friends = json::parse(res->getString("friends"));
                json updated = json::array();
                for (const auto& f : friends) {
                    if (f.value("account", "") != remove_account)
                        updated.push_back(f);
                }

                auto update = conn->prepareStatement("REPLACE INTO friends(account, friends) VALUES (?, ?)");
                update->setString(1, owner);
                update->setString(2, updated.dump());
                update->execute();
            }
        };

        // 2. 双向移除
        remove_from_friend_list(user, target_account);
        remove_from_friend_list(target_account, user);

        // 3. 可选：删除屏蔽记录（如果你在 blocked_friends 表中也存了）
        auto unblock_stmt = conn->prepareStatement("DELETE FROM blocked_friends WHERE blocker = ? AND blocked = ?");
        unblock_stmt->setString(1, user);
        unblock_stmt->setString(2, target_account);
        unblock_stmt->execute();

        auto unblock_stmt2 = conn->prepareStatement("DELETE FROM blocked_friends WHERE blocker = ? AND blocked = ?");
        unblock_stmt2->setString(1, target_account);
        unblock_stmt2->setString(2, user);
        unblock_stmt2->execute();

        response["status"] = "success";
        response["msg"] = "Friend removed successfully";
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}






// ========== 判断是否屏蔽（用于消息发送拦截） ==========
bool is_muted(const std::string& receiver, const std::string& sender) {
    try {
        auto conn = get_mysql_connection();
        auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
        stmt->setString(1, receiver);
        auto res = stmt->executeQuery();

        if (res->next()) {
            json friends = json::parse(res->getString("friends"));
            for (const auto& f : friends) {
                if (f["account"] == sender && f.value("muted", false)) {
                    return true;
                }
            }
        }
    } catch (...) {}
    return false;
}

// ========== 发送私聊消息时调用该方法拦截 ==========
void send_message_if_not_muted(const std::string& sender, const std::string& receiver, const std::string& message) {
    if (is_muted(receiver, sender)) return;
    // 正常发送消息（略）
}

// ========== 推送好友请求通知时判断是否屏蔽 ==========
void notify_friend_request(const std::string& receiver, const json& payload) {
    // 判断是否屏蔽了发送者
    std::string sender = payload.value("from", "");
    if (is_muted(receiver, sender)) return;
    // 推送通知（略）
}




CREATE TABLE friends (
    account VARCHAR(64) PRIMARY KEY,
    friends JSON NOT NULL
);


CREATE TABLE friend_requests (
    id INT PRIMARY KEY AUTO_INCREMENT,
    sender VARCHAR(64) NOT NULL,                               -- 请求发起者
    receiver VARCHAR(64) NOT NULL,                             -- 请求接收者
    status ENUM('pending', 'accepted', 'rejected') NOT NULL
        DEFAULT 'pending',
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    
    INDEX idx_sender_receiver (sender, receiver),
    INDEX idx_receiver_status (receiver, status)
);


[
    {
        "account": "user123",
        "username": "Alice",
        "muted": false
    },
    {
        "account": "user456",
        "username": "Bob",
        "muted": true
    }
]


某好友设置了 muted: true，则不会收到来自该好友的消息或通知


