//
// Created by III on 25-8-18.
//

#include "aes.hpp"
#include "../crypto/CryptoUtils.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <atomic>
#include <nlohmann/json.hpp>

#include "pub.hpp"
using json = nlohmann::json;

// 全局线程池
std::string toHex(const std::vector<uint8_t>& data) {
    static const char* hexChars = "0123456789ABCDEF";
    std::string hex;
    hex.reserve(data.size()*2);
    for (uint8_t b : data) {
        hex.push_back(hexChars[b >> 4]);
        hex.push_back(hexChars[b & 0x0F]);
    }
    return hex;
}


void aesHandler(uWS::HttpResponse<false>* res, uWS::HttpRequest* req) {
    try {
        // 安全持有 HttpResponse
        auto safeRes = std::shared_ptr<uWS::HttpResponse<false>>(res, [](auto*){});
        std::string_view contentType = req->getHeader("content-type");
        if (contentType.find("application/json") == std::string_view::npos) {
            sendJson(safeRes.get(), 400, "invalid content-type, expected application/json");
            return;
        }

        // 捕获 aborted 信号
        auto aborted = std::make_shared<std::atomic_bool>(false);
        res->onAborted([aborted] { aborted->store(true); });
        auto loop = uWS::Loop::get();
        // 读取请求 body
        auto body = std::make_shared<std::string>();
        res->onData([safeRes, body, aborted, loop](std::string_view chunk, bool isLast) {
            body->append(chunk);
            if (!isLast) return;

            auto input = std::make_shared<std::string>(*body);

            // ① 提交到线程池处理
            const auto& pool = controller::getThreadPool();
            boost::asio::post(*pool, [safeRes, input, aborted, loop]() mutable {
                try {
                    json j = json::parse(*input);
                    std::string plaintext = j.value("plaintext", "");
                    if (plaintext.empty()) {
                        loop->defer([safeRes, aborted]() {
                            if (aborted->load()) {
                                sendJson(safeRes.get(), 500, "exception", {{"details", "plaintext.empty()"}});
                            }else {
                                sendJson(safeRes.get(), 400, "missing plaintext", json{});
                            }
                        });
                        return;
                    }

                    // key/iv 可改成从请求里传，示例固定值
                    const CryptoUtils::ByteVec key(32, 'K');
                    const CryptoUtils::ByteVec iv(16, 'I');

                    // AES 加解密
                    auto encrypted = CryptoUtils::aesEncrypt(plaintext, key, iv);
                    auto decrypted = CryptoUtils::aesDecrypt(encrypted, key, iv);

                    json data = {
                        {"encrypted",  CryptoUtils::base64Encode(encrypted)},
                        {"decrypted", std::string(decrypted.begin(), decrypted.end())}
                    };

                    // ② 回到事件循环发送响应
                    loop->defer([safeRes, aborted, data]() {
                        if (aborted->load()) {
                            sendJson(safeRes.get(), 500, "exception", {{"details", "aborted->load()"}});
                        }else {
                            sendJson(safeRes.get(), 200, "OK", data);
                        }
                    });
                } catch (const std::exception& e) {
                    std::cout << e.what() << std::endl;
                    loop->defer([safeRes, aborted, e]() {
                        if (aborted->load()) {
                            sendJson(safeRes.get(), 500, "exception", {{"details", "aborted->load()"}});
                        }else {
                            sendJson(safeRes.get(), 500, "exception", {{"details", e.what()}});
                        }


                    });
                }
            });
        });

    } catch (const std::exception& e) {
        // handler 异常保护
        res->writeStatus("500 Internal Server Error")
           ->end(std::string(R"({"error":"unhandled exception","details":")") + e.what() + "\"}");
    }
}

// io 和cpu密集型任务参考
// int main() {
//     // 1️⃣ 创建io_context用于异步IO任务
//     boost::asio::io_context ioContext;
//
//     // 2️⃣ 创建CPU线程池处理计算密集型任务
//     boost::asio::thread_pool cpuPool(4);
//
//     // 3️⃣ 启动一个线程来跑io_context
//     std::thread ioThread([&ioContext]() {
//         ioContext.run();
//     });
//
//     uWS::App().post("/api/task", [&ioContext, &cpuPool](auto *res, auto *req) {
//         // 获取请求body
//         std::string body = std::string(req->getUrl().value);
//         auto counter = std::make_shared<std::atomic<int>>(2);
//            auto result = std::make_shared<std::string>();
//         // 4️⃣ 提交一个CPU密集型任务到线程池
//         boost::asio::post(cpuPool, [body]() {
//             // 模拟CPU密集计算
//             std::this_thread::sleep_for(std::chrono::milliseconds(50));
//             std::cout << "CPU task finished for: " << body << std::endl;
//                if (counter->fetch_sub(1) == 1) { // 最后一个任务完成
//              res->writeStatus("200 OK")->end(*result);
//          }
//         });
//
//         // 5️⃣ 提交一个IO任务到io_context
//         boost::asio::post(ioContext, [res]() {
//             // 模拟异步IO，比如访问数据库或文件
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//             res->writeStatus("200 OK")->end("Task accepted");
//         });
//
//     }).listen(9000, [](auto *listenSocket) {
//         if (listenSocket) {
//             std::cout << "Server listening on port 9000" << std::endl;
//         }
//     }).run();
//
//     ioThread.join();
//     cpuPool.join();
// }