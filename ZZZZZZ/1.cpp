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
