#include "MessageQueue.hpp"
#include "msg.hpp"

MessageSender::MessageSender() : stop_(false) {
    worker_ = std::thread([this]() { this->process(); });
}

MessageSender::~MessageSender() {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        stop_ = true;
    }
    cond_.notify_one();
    if (worker_.joinable())
        worker_.join();
}

void MessageSender::enqueue(int fd, const json& msg, MsgType type) {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        user_queues_[fd].push({fd, msg, type});
    }
    cond_.notify_one();
}

void MessageSender::process() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock, [this]() { return stop_ || !all_queues_empty(); });

        if (stop_ && all_queues_empty()) break;

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

bool MessageSender::all_queues_empty() {
    for (auto& [fd, q] : user_queues_) {
        if (!q.empty()) return false;
    }
    return true;
}
