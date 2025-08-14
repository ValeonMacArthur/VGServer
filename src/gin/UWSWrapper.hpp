//
// Created by mike on 25-8-14.
//

#pragma once

#include <uWebSockets/App.h>
#include <functional>
#include <string>
#include <algorithm>
#include <iostream>
#include <boost/asio.hpp>
#include <thread>

// 默认空用户数据（类似Java接口的默认实现）
struct EmptySocketData {};

class UWSWrapper {
private:
    uWS::App app;

public:
    UWSWrapper() = default;

    // 添加HTTP路由，类似Gin的风格，返回*this以支持链式调用
    UWSWrapper& addRoute(std::string method, std::string_view path,
                     std::function<void(uWS::HttpResponse<false>*, uWS::HttpRequest*)> handler)
     {
        std::ranges::transform(method, method.begin(), ::toupper);;
        if (method == "GET") {
            app.get(std::string(path), std::move(handler));
        } else if (method == "POST") {
            app.post(std::string(path), std::move(handler));
        } else if (method == "PUT") {
            app.put(std::string(path), std::move(handler));
        } else if (method == "DELETE") {
            app.del(std::string(path), std::move(handler));
        } else if (method == "PATCH") {
            app.patch(std::string(path), std::move(handler));
        } else if (method == "OPTIONS") {
            app.options(std::string(path), std::move(handler));
        } else {
            // 对于根本不支持的 method，用 any 返回 405
            app.any(std::string(path), [](uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
                res->writeStatus("405 Method Not Allowed")->end("Unsupported HTTP method");
            });
        }
        return *this;
    }


    template <typename USERDATA = EmptySocketData>
        UWSWrapper& addWS(const std::string_view path, uWS::TemplatedApp<false>::WebSocketBehavior<USERDATA> behavior) {
        app.ws<USERDATA>(std::string(path), std::move(behavior));
        return *this;
    }


    // 监听端口
    void listen(int port, const std::function<void(us_listen_socket_t*)>& callback = nullptr) {
        app.listen(port, [callback, port](us_listen_socket_t* socket) {
            if (socket) {
                std::cout << "Listening on port " << port << std::endl;
            } else {
                std::cout << "Failed to listen on port " << port << std::endl;
            }
            if (callback) callback(socket);
        });
    }

    // 运行服务器（启动事件循环）
    void run() {
        app.run();
    }
};