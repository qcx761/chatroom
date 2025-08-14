#pragma once
#include <unordered_map>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class MsgType { PRIVATE, GROUP };

struct Message {
    int fd;
    json content;
    MsgType type;
};

// 声明发送函数接口
void send_private_message_msg(int fd, const json& msg);
void send_group_message_msg(int fd, const json& msg);

class MessageSender {
public:
    MessageSender();
    ~MessageSender();

    void enqueue(int fd, const json& msg, MsgType type);

private:
    void process();
    bool all_queues_empty();

    std::unordered_map<int, std::queue<Message>> user_queues_;
    std::mutex mtx_;
    std::condition_variable cond_;
    std::thread worker_;
    bool stop_;
};
