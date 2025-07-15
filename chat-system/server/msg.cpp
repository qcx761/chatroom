#include "json.hpp"
#include "msg.hpp"

using json = nlohmann::json;
using namespace sw::redis;


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

// 验证token是否有效
bool verify_token(const std::string& token, std::string& out_account) {
    try {
        Redis redis("tcp://127.0.0.1:6379");
        std::string token_key = "token:" + token;
        auto val = redis.get(token_key);
        if (val) {
            out_account = *val;
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

        // 检查account是否已存在
        auto check_stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT COUNT(*) FROM users WHERE account = ?"));
        check_stmt->setString(1, account);
        auto res = check_stmt->executeQuery();
        res->next();

        if (res->getInt(1) > 0) {
            response["type"] = "sign_up";
            response["status"] = "fail";
            response["msg"] = "Account already exists";
        } else {
            auto insert_stmt = std::unique_ptr<sql::PreparedStatement>(
                conn->prepareStatement("INSERT INTO users (account, username, password) VALUES (?, ?, ?)"));
            insert_stmt->setString(1, account);
            insert_stmt->setString(2, username);
            insert_stmt->setString(3, password);  // 建议密码哈希存储
            insert_stmt->executeUpdate();

            response["type"] = "sign_up";
            response["status"] = "success";
            response["msg"] = "Registered successfully";
        }
    } catch (const std::exception &e) {
        response["type"] = "sign_up";
        response["status"] = "error";
        response["msg"] = std::string("Exception: ") + e.what();
    }



    int n;
    do{
    n=send_json(fd, response);
    }while(n!=0);



    // send_json(fd, response);
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

        auto stmt = std::unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement("SELECT password FROM users WHERE account = ?"));
        stmt->setString(1, account);
        auto res = stmt->executeQuery();

        if (!res->next()) {
            response["type"] = "log_in";
            response["status"] = "fail";
            response["msg"] = "Account not found";
            send_json(fd, response);
            return;
        }

        std::string stored_pass = res->getString("password");
        if (stored_pass != password) {
            response["type"] = "log_in";
            response["status"] = "fail";
            response["msg"] = "Incorrect password";
            send_json(fd, response);
            return;
        }

        // 密码正确，生成token，存redis
        Redis redis("tcp://127.0.0.1:6379");
        std::string token = generate_token();
        std::string token_key = "token:" + token;
        redis.set(token_key, account);
        redis.expire(token_key, 3600);  // 1小时有效期

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
    do{
    n=send_json(fd, response);
    }while(n!=0);

    // send_json(fd, response);
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


