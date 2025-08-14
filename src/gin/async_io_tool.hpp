#pragma once
#include <boost/asio.hpp>
#include "../concurrentqueue/concurrentqueue.h"
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <stdexcept>
#include <type_traits>

class AsyncIOTool {
public:
    static constexpr size_t DEFAULT_THREAD_POOL_SIZE = std::thread::hardware_concurrency();
    static constexpr size_t DEFAULT_MAX_QUEUE_SIZE   = 1000;

    explicit AsyncIOTool(size_t thread_pool_size = DEFAULT_THREAD_POOL_SIZE,
                         size_t max_queue_size   = DEFAULT_MAX_QUEUE_SIZE)
        : io_context_(),
          work_guard_(boost::asio::make_work_guard(io_context_)),
          task_queue_(max_queue_size),
          stop_queue_processor_(false)
    {
        StartThreadPool(thread_pool_size);
        StartQueueProcessor();
    }

    ~AsyncIOTool() {
        stop_queue_processor_.store(true);
        if (queue_thread_.joinable()) queue_thread_.join();

        io_context_.stop();
        for (auto& t : io_threads_) {
            if (t.joinable()) t.join();
        }
    }

    #warning "post_task 仅用于异步数据库查询/读取任务"
    template <typename Task>
    auto post_task(Task&& task) -> std::future<std::invoke_result_t<Task>>
    {
        using ReturnT = std::invoke_result_t<Task>;
        auto promise = std::make_shared<std::promise<ReturnT>>();
        auto future  = promise->get_future();

        auto wrapped_task = [task = std::forward<Task>(task), promise]() mutable {
            try {
                if constexpr (std::is_void_v<ReturnT>) {
                    task();
                    promise->set_value();
                } else {
                    promise->set_value(task());
                }
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        };

        if (!task_queue_.try_enqueue(std::move(wrapped_task))) {
            promise->set_exception(std::make_exception_ptr(
                std::runtime_error("Read task queue is full")));
        }

        return future;
    }

    boost::asio::io_context& get_io_context() { return io_context_; }

private:
    void StartThreadPool(size_t thread_pool_size) {
        io_threads_.reserve(thread_pool_size);
        for (size_t i = 0; i < thread_pool_size; ++i) {
            io_threads_.emplace_back([this] { io_context_.run(); });
        }
    }

    void StartQueueProcessor() {
        queue_thread_ = std::thread([this] {
            std::function<void()> task;
            while (!stop_queue_processor_.load()) {
                if (task_queue_.try_dequeue(task)) {
                    boost::asio::post(io_context_, std::move(task));
                } else {
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
            }
        });
    }

private:
    boost::asio::io_context io_context_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;

    moodycamel::ConcurrentQueue<std::function<void()>> task_queue_;

    std::vector<std::thread> io_threads_;
    std::thread queue_thread_;
    std::atomic<bool> stop_queue_processor_;
};
