void show_friend_notifications_msg(int fd, const json& request) {
    json response;
    response["type"] = "show_friend_notifications";

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

        // 1. 拉取未处理的好友请求（receiver是自己，且状态为pending）
        std::vector<json> friend_requests;
        {
            auto stmt = conn->prepareStatement(
                "SELECT sender FROM friend_requests WHERE receiver = ? AND status = 'pending'");
            stmt->setString(1, user_account);
            auto res = stmt->executeQuery();

            while (res->next()) {
                std::string sender_account = res->getString("sender");

                // 查用户名
                auto uname_stmt = conn->prepareStatement(
                    "SELECT JSON_UNQUOTE(JSON_EXTRACT(info, '$.username')) AS username FROM users WHERE JSON_UNQUOTE(JSON_EXTRACT(info, '$.account')) = ?");
                uname_stmt->setString(1, sender_account);
                auto uname_res = uname_stmt->executeQuery();

                std::string sender_username = "";
                if (uname_res->next()) {
                    sender_username = uname_res->getString("username");
                }

                friend_requests.push_back({
                    {"account", sender_account},
                    {"username", sender_username}
                });
            }
        }

        // 2. 获取好友列表
        std::vector<json> friends_list;
        {
            auto stmt = conn->prepareStatement("SELECT friends FROM friends WHERE account = ?");
            stmt->setString(1, user_account);
            auto res = stmt->executeQuery();

            if (res->next()) {
                std::string friends_str = res->getString("friends");
                friends_list = json::parse(friends_str).get<std::vector<json>>();
            }
        }

        // 3. 获取好友文件信息（如果有相关表，假设是 friend_files 表）
        // 这里假设你有一张 friend_files 表：owner（账号）、filename、shared(boolean)
        std::vector<json> friend_files;
        {
            // 先取所有好友账号
            std::vector<std::string> friend_accounts;
            for (const auto& f : friends_list) {
                friend_accounts.push_back(f.value("account", ""));
            }

            if (!friend_accounts.empty()) {
                // 构造IN查询字符串
                std::string in_clause = "(";
                for (size_t i = 0; i < friend_accounts.size(); ++i) {
                    in_clause += "?";
                    if (i != friend_accounts.size() - 1) in_clause += ",";
                }
                in_clause += ")";

                std::string sql = "SELECT owner, filename, shared FROM friend_files WHERE owner IN " + in_clause;

                auto stmt = conn->prepareStatement(sql);
                for (size_t i = 0; i < friend_accounts.size(); ++i) {
                    stmt->setString(i + 1, friend_accounts[i]);
                }
                auto res = stmt->executeQuery();

                while (res->next()) {
                    friend_files.push_back({
                        {"owner", res->getString("owner")},
                        {"filename", res->getString("filename")},
                        {"shared", res->getBoolean("shared")}
                    });
                }
            }
        }

        response["status"] = "success";
        response["friend_requests"] = friend_requests;
        response["friends"] = friends_list;
        response["friend_files"] = friend_files;
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["msg"] = e.what();
    }

    send_json(fd, response);
}



















































void error_msg(int fd, const nlohmann::json &request){
    json response;
    response["type"] = "error";
    response["msg"] = "Unrecognized request type";
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


