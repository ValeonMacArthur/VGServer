//
// Created by III on 2025/7/27.
//
#include "controller.hpp"
#include <spdlog/spdlog.h>
namespace controller {
    void helloHandler(HttpResponse *res, HttpRequest *req) {
        res->writeHeader("Content-Type", "text/plain");
        res->end("Hello from uWS HTTP server!");
    }

    void userHandler(HttpResponse *res, HttpRequest *req) {
        std::string userId = std::string(req->getParameter(0));
        res->end("User ID: " + userId);

        
    }
}