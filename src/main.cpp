//
// Created by III on 2025/7/27.
//

#include <iostream>
#include <spdlog/spdlog.h>
#include "utils/log.hpp"
#include "gin/UWSWrapper.hpp"
#include "database/db_pool.hpp"
#include <csignal>
#include "controller/hello.hpp"
#include "controller/pub.hpp"
#include "controller/aes.hpp"


void signal_handler(int) {
    std::cout << "app shutdown" << std::endl;
}

//
// # 启动 Docker 容器运行 Web 服务，并分配 CPU 核心
// #
// # 分析：
// # 1. uWebSockets 主循环       -> 占用 1 核
// # 2. 异步 Logger            -> 占用 1 核
// # 3. libzdb 数据库线程      -> 占用 1 核
// # 4. boost::asio 线程池(4)  -> 占用 4 核（假设线程池主要处理 CPU 密集任务）
// # 5. 总计                   -> 7 核，预留 1 核冗余
// #
// # 因此建议给容器分配 8 核，以保证高并发下稳定运行
//

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    // 1️⃣ 初始化 日志
    init_logger(LogMode::Dev);
    // 2️ 初始化 CPU 线程池
    controller::getThreadPool() = std::make_unique<boost::asio::thread_pool>(4);
    // 3️⃣ 启动 IO context 单独线程
    auto guard = boost::asio::make_work_guard(controller::getIoContext());
    std::thread ioThread([]{
        controller::getIoContext().run();
    });
    // 提交 CPU 任务
    const auto& pool = controller::getThreadPool();
    boost::asio::post(*pool, []{
        spdlog::info("当前项目线程池: {CPU task running}");
    });

    // 提交 IO 任务
    boost::asio::post(controller::getIoContext(), []{
        spdlog::info("当前项目线程池: {io task running}");
    });

    std::string loop_type;
#if defined(LIBUS_USE_LIBUV)
    loop_type = "libuv";
#elif defined(LIBUS_USE_EPOLL)
    loop_type = "epoll";
#elif defined(LIBUS_USE_KQUEUE)
    loop_type = "kqueue";
#elif defined(LIBUS_USE_ASIO)
    loop_type = "ASIO";
#elif defined(LIBUS_USE_GCD)
    loop_type = "GCD";
#else
    loop_type = "未知（可能自定义或默认未定义）";
#endif
    spdlog::info("当前项目 uWebSockets 底层使用的事件循环类型: {}", loop_type);

    UWSWrapper server;
    server.addRoute("GET", "/hello", helloHandler);
    server.addRoute("POST", "/api/aes", aesHandler);

    server.listen(8080);
    server.run();

    // 4️⃣ 清理
    controller::getThreadPool()->join();
    controller::getIoContext().stop();
    ioThread.join();

    spdlog::shutdown();
    return 0;
}
