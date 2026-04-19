//
// Created by jianp on 2026/3/17.
//
// 这个线程池的实现是我看过最简单也最复杂的

#ifndef CPPKIT_THREADPOOL_H
#define CPPKIT_THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>

class ThreadPool {
public:
    ThreadPool(size_t);

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;
    ~ThreadPool();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};

inline ThreadPool::ThreadPool(size_t threads)
    : stop(false) {
    for(size_t i = 0;i < threads; i++) {
        workers.emplace_back(
                [this] {
                    for(;;) {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock, [this]{
                                return this->stop.load() || !this->tasks.empty();
                            });
                            if(this->stop.load() && this->tasks.empty()) {
                                return ;
                            }
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }

                        task();
                    }
                }
                );
    }
}

template<class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>> {
    using returnType = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<returnType()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

    std::future<returnType> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if(stop.load()) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        tasks.emplace([task](){ (*task)();});
    }

    condition.notify_one();
    return res;
}

inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop.store(true);
    }

    condition.notify_all();
    for(std::thread& worker : workers) {
        worker.join();
    }
}

#endif //CPPKIT_THREADPOOL_H
