#pragma once

#include <condition_variable>
#include <deque>
#include <future>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(int num_of_threads);

    template <class T, class... Args>
    auto enqueue(T &&f, Args &&...args) -> std::optional<std::future<std::invoke_result_t<T, Args...>>>;

    ~ThreadPool();

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::thread> workers_;
    std::deque<std::packaged_task<void(void)>> tasks_;
    bool stop_{false};
};

ThreadPool::ThreadPool(int num_of_threads) {
    for (int i = 0; i < num_of_threads; i++) {
        workers_.emplace_back([this]() {
            while (true) {
                std::packaged_task<void()> task;
                {
                    std::unique_lock lock(mutex_);
                    cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                    if (stop_ && tasks_.empty())
                        return;
                    task = std::move(tasks_.front());
                    tasks_.pop_front();
                }

                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cv_.notify_all();
    for (std::thread &thread : workers_) {
        if (thread.joinable())
            thread.join();
    }
}

template <class T, class... Args>
auto ThreadPool::enqueue(T &&f, Args &&...args) -> std::optional<std::future<std::invoke_result_t<T, Args...>>> {
    using return_type = std::invoke_result_t<T, Args...>;

    std::packaged_task<return_type()> task(std::bind(std::forward<T>(f), std::forward<Args>(args)...));
    auto future = task.get_future();
    {
        std::unique_lock lock(mutex_);

        // don't allow enqueueing after stopping the pool
        if (stop_) {
            return std::nullopt;
        }

        tasks_.emplace_back(std::move(task));
    }
    cv_.notify_one();
    return std::make_optional(std::move(future));
}
