//
// Created by mike on 25-8-12.
//
#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <vector>
#include "../database/db_pool.hpp"

static constexpr auto host = "host.docker.internal";
static constexpr int port = 5432;
static constexpr auto dbname = "mydatabase";
static constexpr auto user = "postgres";
static constexpr auto password = "Rg9vTzXr82bqLpNfU3nF";

TEST_CASE("DBPool initialization and single connection test", "[dbpool]") {
    DBPool::init_postgres(host, port, dbname, user, password);
    REQUIRE(DBPool::test_connection() == true);
    DBPool::shutdown();
}

TEST_CASE("DBPool single thread + one std::thread test", "[dbpool][thread]") {
    DBPool::init_postgres(host, port, dbname, user, password, 10, 20, 5);

    // 10个线程并发测试连接
    const int thread_count = 10;
    std::vector<std::thread> threads;
    std::vector<bool> results(thread_count, false);

    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&results, i]() {
            results[i] = DBPool::test_connection();
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    for (bool res : results) {
        REQUIRE(res == true);
    }

    DBPool::shutdown();
}


TEST_CASE("DBPool multi-threaded concurrent connection test", "[dbpool][concurrent]") {
    DBPool::init_postgres(host, port, dbname, user, password);

    const int thread_count = 3;
    std::vector<std::thread> threads;
    std::vector<bool> results(thread_count, false);

    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&results, i]() {
            results[i] = DBPool::test_connection();
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    for (bool res : results) {
        REQUIRE(res == true);
    }

    DBPool::shutdown();
}