#pragma once

#include <nlohmann/json.hpp>
#include <sw/redis++/redis++.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <random>
#include <string.h>
#include <sstream>
#include <memory>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <thread>


using json = nlohmann::json;
using namespace sw::redis;

extern sw::redis::Redis redis;

void error_msg(int fd, const json &request);
std::shared_ptr<sql::Connection> get_mysql_connection();
std::string generate_token();
bool verify_token(const std::string& token, std::string& out_account);
void sign_up_msg(int fd, const json &request);
void log_in_msg(int fd, const json &request);
void destory_account_msg(int fd, const json &request);
void quit_account_msg(int fd, const json &request);
void username_view_msg(int fd, const json &request);
void username_change_msg(int fd, const json &request);
void password_change_msg(int fd, const json &request);
void show_friend_list_msg(int fd, const json &request);
void add_friend_msg(int fd, const json &request);
void mute_friend_msg(int fd, const json &request);
void unmute_friend_msg(int fd, const json &request);
void remove_friend_msg(int fd, const json &request);
void add_friend_msg(int fd, const json &request);
void handle_friend_request_msg(int fd, const json &request);
void get_friend_requests_msg(int fd, const json& request);
void get_friend_info_msg(int fd, const json& request);
void send_private_message_msg(int fd, const json& request);
void get_private_history_msg(int fd, const json& request);
void get_unread_private_messages_msg(int fd, const json& request);
void show_group_list_msg(int fd, const json& request);
void join_group_msg(int fd, const json& request);
void quit_group_msg(int fd, const json& request);
void show_group_members_msg(int fd, const json& request);
void create_group_msg(int fd, const json& request);
void set_group_admin_msg(int fd, const json& request);
void remove_group_admin_msg(int fd, const json& request);
void remove_group_member_msg(int fd, const json& request);
void add_group_member_msg(int fd, const json& request);
void dismiss_group_msg(int fd, const json& request);
void get_unread_group_messages_msg(int fd, const json& request);
void get_group_history_msg(int fd, const json& request);
void send_group_message_msg(int fd, const json& request);
void get_group_requests_msg(int fd, const json& request);
void handle_group_request_msg(int fd, const json& request);
void send_group_file_msg(int fd, const json& request);
void send_private_file_msg(int fd, const json& request);
void get_file_list_msg(int fd, const json& request);
void send_offline_summary_on_login(const std::string& account, int fd);
bool redis_key_exists(const std::string &token);
void refresh_online_status(const std::string &token);
std::string getCurrentTimeString();




class MySQLPool {
public:
    MySQLPool(const std::string& url, const std::string& user, const std::string& password,
              const std::string& schema, int pool_size)
        : url_(url), user_(user), password_(password), schema_(schema), pool_size_(pool_size)
    {
        driver_ = sql::mysql::get_mysql_driver_instance();
        if (!driver_) throw std::runtime_error("Failed to get MySQL driver");

        // 预先创建连接放入池
        for (int i = 0; i < pool_size_; ++i) {
            sql::Connection* conn = driver_->connect(url_, user_, password_);
            conn->setSchema(schema_);
            connections_.push(conn);
        }
    }

    ~MySQLPool() {
        std::lock_guard<std::mutex> lock(mtx_);
        while (!connections_.empty()) {
            sql::Connection* conn = connections_.front();
            connections_.pop();
            conn->close();
            delete conn;
        }
    }

    // 获取连接，返回shared_ptr，析构时自动归还连接池
    std::shared_ptr<sql::Connection> getConnection() {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock, [this]() { return !connections_.empty(); });
        sql::Connection* conn = connections_.front();
        connections_.pop();

        // 自定义删除器，shared_ptr析构时会把连接归还给池
        return std::shared_ptr<sql::Connection>(conn, [this](sql::Connection* c) {
            std::lock_guard<std::mutex> lock(this->mtx_);
            this->connections_.push(c);
            this->cond_.notify_one();
        });
    }

private:
    std::string url_, user_, password_, schema_;
    int pool_size_;
    sql::mysql::MySQL_Driver* driver_;

    std::queue<sql::Connection*> connections_;
    std::mutex mtx_;
    std::condition_variable cond_;
};

struct ChatMessage {
    std::string sender;
    std::string receiver;
    std::string content;
    bool is_online;
};

class AsyncMessageInserter {
public:
    AsyncMessageInserter(MySQLPool& pool) : pool_(pool), stop_(false) {
        worker_ = std::thread(&AsyncMessageInserter::workerLoop, this);
    }

    ~AsyncMessageInserter() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            stop_ = true;
        }
        cv_.notify_all();
        if (worker_.joinable()) worker_.join();
    }

    // 主线程调用：非阻塞入队
    void enqueueMessage(const std::string& sender,
                        const std::string& receiver,
                        const std::string& content,
                        bool is_online) 
    {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.push({sender, receiver, content, is_online});
        }
        cv_.notify_one();
    }

private:
    void workerLoop() {
        while (true) {
            ChatMessage msg;
            {
                std::unique_lock<std::mutex> lock(mtx_);
                cv_.wait(lock, [this]() { return stop_ || !queue_.empty(); });
                if (stop_ && queue_.empty()) break;
                msg = queue_.front();
                queue_.pop();
            }

            try {
                auto conn = pool_.getConnection();
                std::unique_ptr<sql::PreparedStatement> stmt(
                    conn->prepareStatement(
                        "INSERT INTO messages "
                        "(sender, receiver, content, is_online, is_read) "
                        "VALUES (?, ?, ?, ?, FALSE)"));
                stmt->setString(1, msg.sender);
                stmt->setString(2, msg.receiver);
                stmt->setString(3, msg.content);
                stmt->setBoolean(4, msg.is_online);
                stmt->execute();
            } catch (const sql::SQLException& e) {
                std::cerr << "[AsyncMessageInserter] Insert failed: " << e.what() << std::endl;
                // 可加入重试逻辑
            }
        }
    }

    MySQLPool& pool_;
    std::queue<ChatMessage> queue_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::thread worker_;
    bool stop_;
};




struct GroupMessage {
    int group_id;
    std::string sender;
    std::string content;
};
class AsyncGroupMessageInserter {
public:
    AsyncGroupMessageInserter(MySQLPool& pool) : pool_(pool), stop_(false) {
        worker_ = std::thread(&AsyncGroupMessageInserter::workerLoop, this);
    }

    ~AsyncGroupMessageInserter() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            stop_ = true;
        }
        cv_.notify_all();
        if (worker_.joinable()) worker_.join();
    }

    // 主线程调用：非阻塞入队
    void enqueueMessage(int group_id, const std::string& sender, const std::string& content) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.push({group_id, sender, content});
        }
        cv_.notify_one();
    }

private:
    void workerLoop() {
        while (true) {
            GroupMessage msg;
            {
                std::unique_lock<std::mutex> lock(mtx_);
                cv_.wait(lock, [this]() { return stop_ || !queue_.empty(); });
                if (stop_ && queue_.empty()) break;
                msg = queue_.front();
                queue_.pop();
            }

            try {
                auto conn = pool_.getConnection();
                std::unique_ptr<sql::PreparedStatement> stmt(
                    conn->prepareStatement(
                        "INSERT INTO group_messages (group_id, sender, content) VALUES (?, ?, ?)"));
                stmt->setInt(1, msg.group_id);
                stmt->setString(2, msg.sender);
                stmt->setString(3, msg.content);
                stmt->executeUpdate();
            } catch (const sql::SQLException& e) {
                std::cerr << "[AsyncGroupMessageInserter] Insert failed: " << e.what() << std::endl;
            }
        }
    }

    MySQLPool& pool_;
    std::queue<GroupMessage> queue_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::thread worker_;
    bool stop_;
};












enum class MsgType { PRIVATE, GROUP };

struct Message {
    int fd;             
    json content;
    MsgType type;
};

class MessageSender {
public:
    MessageSender() : stop_(false) {
        worker_ = std::thread([this]() { this->process(); });
    }

    ~MessageSender() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            stop_ = true;
        }
        cond_.notify_one();
        worker_.join();
    }

    // 保持原接口不变
    void enqueue(int fd, const json& msg, MsgType type) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            user_queues_[fd].push({fd, msg, type});
        }
        cond_.notify_one();
    }

private:
    void process() {
        while (true) {
            std::unique_lock<std::mutex> lock(mtx_);
            cond_.wait(lock, [this]() { return stop_ || !all_queues_empty(); });

            if (stop_ && all_queues_empty()) break;

            // 遍历每个 fd 队列，发送一条消息
            for (auto& [fd, q] : user_queues_) {
                if (!q.empty()) {
                    Message msg = q.front();
                    q.pop();
                    lock.unlock(); // 发送可能阻塞，先解锁
                    if (msg.type == MsgType::PRIVATE)
                        send_private_message_msg(msg.fd, msg.content);
                    else
                        send_group_message_msg(msg.fd, msg.content);
                    lock.lock();
                }
            }
        }
    }

    bool all_queues_empty() {
        for (auto& [fd, q] : user_queues_) {
            if (!q.empty()) return false;
        }
        return true;
    }

    std::unordered_map<int, std::queue<Message>> user_queues_;
    std::mutex mtx_;
    std::condition_variable cond_;
    std::thread worker_;
    bool stop_;
};














