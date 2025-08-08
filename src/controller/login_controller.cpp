//
// Created by III on 25-7-30.
//
#include "controller.hpp"
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/futures/Future.h>

extern std::shared_ptr<folly::CPUThreadPoolExecutor> threadPool;
namespace controller {
    struct AsyncContext {
        std::string result;
        std::shared_ptr<HttpResponse> res;
        std::atomic<bool> aborted = false;
    };

    void verifyCodeHandler(HttpResponse *res, HttpRequest *req) {
        res->pause();  // 暂停响应，后续异步恢复

        auto context = std::make_shared<controller::AsyncContext>();
        // 用shared_ptr管理res，不释放res指针（deleter空）
        context->res = std::shared_ptr<HttpResponse>(res, [](HttpResponse*) {});

        // 监听连接是否中断
        res->onAborted([context]() {
            context->aborted.store(true, std::memory_order_relaxed);
        });

        // 异步任务
        folly::via(threadPool.get(), [context]() {
            // 模拟耗时任务
            std::cerr << "Task start in thread " << std::this_thread::get_id() << std::endl;
        context->result = R"({"message":"Hello from async controller!"})";
        std::cerr << "Task done" << std::endl;
        }).thenValue([context](auto&&) {
            if (context->aborted.load(std::memory_order_relaxed) || !context->res) {
                // 如果连接已中断，什么都不做
                return;
            }
            context->res->writeHeader("Content-Type", "application/json");
            context->res->end(context->result);
        }).thenError(folly::tag_t<std::exception>{}, [context](const std::exception& e) {
            if (context->aborted.load(std::memory_order_relaxed) || !context->res) {
                return;
            }
            context->res->writeStatus("500 Internal Server Error");
            context->res->end(std::string(R"({"error":")") + e.what() + R"("})");
        });
    }
}