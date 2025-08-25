//
// Created by III on 25-8-17.
//

#ifndef PUB_HPP
#define PUB_HPP
#include <uWebSockets/App.h>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>

namespace controller {
    inline std::unique_ptr<boost::asio::thread_pool>& getThreadPool() {
        static std::unique_ptr<boost::asio::thread_pool> pool;
        return pool;
    }
    // IO context 用于处理异步 IO
    inline boost::asio::io_context& getIoContext() {
        static boost::asio::io_context ioContext;
        return ioContext;
    }
}

inline const char* getStatusText(int code) {
    switch (code) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
}

inline void sendJson(uWS::HttpResponse<false>* res, int code,
                     std::string_view message, nlohmann::json data = nullptr)
{
    try {
        nlohmann::json j;
        j["code"] = code;           // 数字 code
        j["message"] = message;     // 文字信息
        j["data"] = std::move(data);

        res->writeHeader("Content-Type", "application/json")
           ->writeStatus("200 OK")
           ->end(j.dump());
    } catch (const std::exception& e) {
        std::cerr << "sendJson exception: " << e.what() << "\n";
        res->writeStatus("500 Internal Server Error")
           ->end(R"({"code":500,"message":"Internal Server Error"})");
    }
}

#endif //PUB_HPP
