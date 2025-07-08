#include <iostream>
#include <unordered_map>
#include <functional>

int main() {
    std::unordered_map<int, std::function<void(int)>> callbacks;

    // 给fd=5注册一个回调处理事件A
    callbacks[5] = [](int fd) {
        std::cout << "处理事件A，fd=" << fd << std::endl;
    };

    // 给fd=10注册另一个回调处理事件B
    callbacks[10] = [](int fd) {
        std::cout << "处理事件B，fd=" << fd << std::endl;
    };

    // 模拟事件循环，收到事件fd=5和fd=10
    int events[] = {5, 10};

    for (int fd : events) {
        if (callbacks.count(fd)) {
            // 事件发生，调用注册的回调函数
            callbacks[fd](fd);
        }
    }

    return 0;
}




































#include <iostream>
#include <unordered_map>
#include <functional>
#include <queue>
#include <thread>
#include <chrono>

// 模拟事件类型
enum class EventType {
    Read,
    Write,
    Close,
};

// 事件结构体
struct Event {
    int fd;
    EventType type;
};

// 全局回调表：fd + 事件类型 => 处理函数
std::unordered_map<int, std::function<void(int)>> readCallbacks;
std::unordered_map<int, std::function<void(int)>> writeCallbacks;
std::unordered_map<int, std::function<void(int)>> closeCallbacks;

// 模拟事件队列（线程安全性这里不考虑，简化） 
std::queue<Event> eventQueue;

// 模拟事件循环，不断从事件队列里取事件，调用对应回调
void eventLoop() {
    while (true) {
        if (!eventQueue.empty()) {
            Event ev = eventQueue.front();
            eventQueue.pop();

            switch (ev.type) {
                case EventType::Read:
                    if (readCallbacks.count(ev.fd)) {
                        readCallbacks[ev.fd](ev.fd);
                    }
                    break;
                case EventType::Write:
                    if (writeCallbacks.count(ev.fd)) {
                        writeCallbacks[ev.fd](ev.fd);
                    }
                    break;
                case EventType::Close:
                    if (closeCallbacks.count(ev.fd)) {
                        closeCallbacks[ev.fd](ev.fd);
                    }
                    break;
            }
        } else {
            // 没事件，稍微休息，模拟异步等待
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

int main() {
    // 注册回调函数
    readCallbacks[1] = [](int fd){
        std::cout << "fd " << fd << " 读事件处理: 读取数据" << std::endl;
    };
    writeCallbacks[1] = [](int fd){
        std::cout << "fd " << fd << " 写事件处理: 发送数据" << std::endl;
    };
    closeCallbacks[1] = [](int fd){
        std::cout << "fd " << fd << " 关闭事件处理: 清理资源" << std::endl;
    };

    // 启动事件循环（放在另一个线程，模拟后台运行）
    std::thread loopThread(eventLoop);

    // 模拟异步事件发生
    eventQueue.push({1, EventType::Read});
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    eventQueue.push({1, EventType::Write});
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    eventQueue.push({1, EventType::Close});
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 主线程睡一会儿，等事件处理完
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 程序退出（为了演示，简单退出，不做清理）
    std::terminate();
}
