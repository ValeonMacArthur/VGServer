//
// Created by III on 2025/7/27.
//

#include <iostream>
#include <spdlog/spdlog.h>
#include "utils/log.hpp"
#include "gin/UWSWrapper.hpp"
#include "database/db_pool.hpp"
#include <csignal>
static constexpr auto host = "host.docker.internal";
static constexpr int port = 5432;
static constexpr auto dbname = "mydatabase";
static constexpr auto user = "postgres";
static constexpr auto password = "Rg9vTzXr82bqLpNfU3nF";
static constexpr auto initial_conns = 10;
static constexpr auto max_conns = 20;

void signal_handler(int) {
    std::cout << "app shutdown"  << std::endl;
}

// 可以定义在 main 外面
void helloHandler(uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
    res->end("Hello, world from outside main!");
}

// 其他 handler 也可以类似定义
void goodbyeHandler(uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
    res->end("Goodbye!");
}


int main(int argc, char** argv)  {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    init_logger(LogMode::Dev);

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
spdlog::info("当前项目 uWebSockets 底层使用的事件循环类型: {}",  loop_type);

    UWSWrapper server;
    server.addRoute("GET", "/hello", helloHandler);
    server.listen(8080);
    server.run();


    spdlog::shutdown();
    return 0;

}