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









CREATE TABLE private_messages (
    id INT PRIMARY KEY AUTO_INCREMENT,
    sender VARCHAR(64) NOT NULL,
    receiver VARCHAR(64) NOT NULL,
    content TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);













#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

using json = nlohmann::json;
using namespace std;

#define MAXBUF 1024

// 简单打印和刷新，模拟rl_on_new_line和rl_redisplay功能
void print_message(const string& msg) {
    cout << "\r\033[K"; // 清除当前行
    cout << msg << endl;
    cout << "> " << flush; // 打印提示符并刷新
}

// 收消息线程函数
void recv_thread_func(int sockfd, atomic<bool>& running) {
    char buf[MAXBUF];
    string buffer;

    while (running) {
        ssize_t n = recv(sockfd, buf, MAXBUF, 0);
        if (n > 0) {
            buffer.append(buf, n);

            // 简单假设消息以'\n'分割（实际你用长度前缀包更好）
            size_t pos;
            while ((pos = buffer.find('\n')) != string::npos) {
                string line = buffer.substr(0, pos);
                buffer.erase(0, pos + 1);

                try {
                    json recvjson = json::parse(line);
                    if (recvjson["sort"] == "MESSAGE" && recvjson["request"] == "PEER_CHAT") {
                        string sender = recvjson["sender"];
                        string message = recvjson["message"];
                        print_message("[" + sender + "]: " + message);
                    }
                } catch (...) {
                    cerr << "JSON parse error\n";
                }
            }
        } else if (n == 0) {
            cout << "Server closed connection\n";
            running = false;
        } else {
            perror("recv");
            running = false;
        }
    }
}

// 发送私聊消息函数
void send_peer_chat(int sockfd, const string& sender, const string& receiver, const string& message) {
    json sendjson = {
        {"sort", "MESSAGE"},
        {"request", "PEER_CHAT"},
        {"sender", sender},
        {"receiver", receiver},
        {"message", message}
    };
    string sendstr = sendjson.dump() + "\n"; // 以换行符结束
    send(sockfd, sendstr.c_str(), sendstr.size(), 0);
}

int main() {
    // 伪代码示例，假设已连接服务器sockfd
    int sockfd = /* connect to server socket */;
    atomic<bool> running(true);

    // 开收消息线程
    thread recv_thread(recv_thread_func, sockfd, std::ref(running));

    string username = "alice";  // 本人名字
    string chat_with = "bob";   // 聊天对象

    while (running) {
        cout << "> " << flush;
        string line;
        if (!getline(cin, line)) break;

        if (line == "/quit") break;

        send_peer_chat(sockfd, username, chat_with, line);
    }

    running = false;
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    recv_thread.join();

    return 0;
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