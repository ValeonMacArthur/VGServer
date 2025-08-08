//
// Created by III on 2025/7/27.
//
#include "controller.hpp"
#include <spdlog/spdlog.h>
namespace controller {
    void helloHandler(HttpResponse *res, HttpRequest *req) {
        spdlog::debug("这条是 debug 级别");
        spdlog::warn("warn 警告");
        spdlog::error("error 错误");
        res->writeHeader("Content-Type", "text/plain");
        res->end("Hello from uWS HTTP server!");
    }

    void userHandler(HttpResponse *res, HttpRequest *req) {
        std::string userId = std::string(req->getParameter(0));
        res->end("User ID: " + userId);

        
    }
}