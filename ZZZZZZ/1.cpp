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

    try {
        auto conn = get_mysql_connection();
        auto stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT id, info FROM users WHERE JSON_EXTRACT(info, '$.account') = ?"));
        stmt->setString(1, account);
        auto res = stmt->executeQuery();

        if (!res->next()) {
            response["type"] = "destory_account";
            response["status"] = "fail";
            response["msg"] = "Account not found";
        } else {
            std::string info_str = res->getString("info");
            json user_info = json::parse(info_str);
            if (user_info.value("password", "") != password) {
                response["type"] = "destory_account";
                response["status"] = "fail";
                response["msg"] = "Incorrect password";
            } else {
                int id = res->getInt("id");
                auto del_stmt = std::unique_ptr<sql::PreparedStatement>(
                    conn->prepareStatement("DELETE FROM users WHERE id = ?"));
                del_stmt->setInt(1, id);
                del_stmt->executeUpdate();

                Redis redis("tcp://127.0.0.1:6379");
                redis.del("token:" + token);

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
            json user_info = json::parse(res->getString("info"));
            response["type"] = "username_view";
            response["status"] = "success";
            response["username"] = user_info["username"];
        } else {
            response["type"] = "username_view";
            response["status"] = "fail";
            response["msg"] = "Account not found";
        }
    } catch (const std::exception &e) {
        response["type"] = "username_view";
        response["status"] = "error";
        response["msg"] = e.what();
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
            json info = json::parse(res->getString("info"));
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
            json info = json::parse(res->getString("info"));
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
