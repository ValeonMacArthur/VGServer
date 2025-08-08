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
#include <thread>
std::atomic<bool> running = true;
void signal_handler(int signal) {
    spdlog::info("Signal {} received, shutting down...", signal);
    running = false;
}

std::shared_ptr<folly::CPUThreadPoolExecutor> threadPool;


int main() {
    init_logger(LogMode::Dev);
    threadPool = std::make_shared<folly::CPUThreadPoolExecutor>(8);
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