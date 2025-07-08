#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <type_traits>

class threadpool {
public:
    explicit threadpool(size_t num);
    ~threadpool();

    template<typename F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::condition_variable condition;
    std::mutex mtx;
    bool stop;
};

// 模板函数必须放在头文件
template<typename F, class... Args>
auto threadpool::enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    std::future<return_type> res = task->get_future();

    {
        std::unique_lock<std::mutex> lock(this->mtx);
        if (this->stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        this->tasks.emplace([task]() { (*task)(); });
    }
    this->condition.notify_one();
    return res;
}

