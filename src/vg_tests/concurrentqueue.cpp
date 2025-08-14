//
// Created by mike on 25-8-14.
//
#include <catch2/catch_test_macros.hpp>
#include "../concurrentqueue/concurrentqueue.h"
#include <thread>
#include <vector>

TEST_CASE("ConcurrentQueue basic operations", "[concurrentqueue]") {
    moodycamel::ConcurrentQueue<int> queue;

    SECTION("Enqueue and Dequeue single item") {
        queue.enqueue(42);

        int value = 0;
        REQUIRE(queue.try_dequeue(value));
        REQUIRE(value == 42);

        // Queue should now be empty
        REQUIRE_FALSE(queue.try_dequeue(value));
    }

    SECTION("Enqueue and Dequeue multiple items") {
        for (int i = 0; i < 5; ++i)
            queue.enqueue(i);

        for (int i = 0; i < 5; ++i) {
            int value = -1;
            REQUIRE(queue.try_dequeue(value));
            REQUIRE(value == i);
        }

        int dummy;
        REQUIRE_FALSE(queue.try_dequeue(dummy));
    }
}

TEST_CASE("ConcurrentQueue multi-threaded test", "[concurrentqueue][multithread]") {
    moodycamel::ConcurrentQueue<int> queue;

    const int numThreads = 4;
    const int itemsPerThread = 1000;

    std::atomic<int> consumedCount{0};
    std::vector<int> consumed(numThreads * itemsPerThread, -1); // 存储消费的值

    // 启动生产者线程
    std::vector<std::thread> producers;
    for (int t = 0; t < numThreads; ++t) {
        producers.emplace_back([&, t]() {
            for (int i = 0; i < itemsPerThread; ++i) {
                queue.enqueue(t * itemsPerThread + i);
            }
        });
    }

    // 启动消费者线程
    std::thread consumer([&]() {
        int value;
        while (consumedCount.load() < numThreads * itemsPerThread) {
            while (queue.try_dequeue(value)) {
                REQUIRE(value >= 0);
                REQUIRE(value < numThreads * itemsPerThread);
                consumed[value] = 1; // 标记已消费
                consumedCount.fetch_add(1);
            }
            std::this_thread::yield(); // 避免忙等
        }
    });

    // 等待所有线程完成
    for (auto& t : producers) t.join();
    consumer.join();

    // 检查所有值都被消费
    for (int i = 0; i < numThreads * itemsPerThread; ++i) {
        REQUIRE(consumed[i] == 1);
    }

    // 最终计数检查
    REQUIRE(consumedCount.load() == numThreads * itemsPerThread);
}