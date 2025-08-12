//
// Created by III on 2025/7/27.
//

#include <uWebSockets/App.h>
#include <iostream>
#include "controller/router.hpp"
#include <spdlog/spdlog.h>
#include "utils/log.hpp"
#include <atomic>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/futures/Future.h>
#include <folly/init/Init.h>
#include <thread>

std::atomic<bool> running = true;
void signal_handler(int signal) {
    spdlog::info("Signal {} received, shutting down...", signal);
    running = false;
}

std::shared_ptr<folly::CPUThreadPoolExecutor> threadPool;


int main(int argc, char** argv)  {
    folly::Init init(&argc, &argv);

    unsigned int total_cpus = std::thread::hardware_concurrency();
    if (total_cpus == 0) {
        total_cpus = 1; // fallback
    }
    // 预留 1 个给 uWebSockets 主线程
    // 预留 1 个给 spdlog
    // 预留 1 个给 libzdb
    unsigned int reserve_cpus = 3;
    unsigned int pool_threads = (total_cpus > reserve_cpus) ? total_cpus - reserve_cpus : 1;

    auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(
        pool_threads,
        std::make_shared<folly::NamedThreadFactory>("http-worker")
    );


    init_logger(LogMode::Dev);

    uWS::App app;
    setup_routes(app);
    app.listen(8080, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "[OK] HTTP server listening on port 8080\n";
        } else {
            std::cerr << "[ERROR] Failed to start HTTP server\n";
        }
    }).run();
    // 等待退出信号
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    // 关闭日志系统
    spdlog::shutdown();
    return 0;

}