//
// Created by III on 2025/7/27.
//

#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <filesystem>
#include <memory>
#include <string>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/details/log_msg.h>
#include <chrono>
enum class LogMode {
    Dev,
    PreRelease,
    Release
};

class unix_ms_formatter final : public spdlog::custom_flag_formatter {
public:
    void format(const spdlog::details::log_msg& msg, const std::tm&, spdlog::memory_buf_t& dest) override {
        using namespace std::chrono;
        auto ms = duration_cast<milliseconds>(msg.time.time_since_epoch()).count();
        fmt::format_to(std::back_inserter(dest), "{}", ms);
    }
    [[nodiscard]] std::unique_ptr<custom_flag_formatter> clone() const override {
        return std::make_unique<unix_ms_formatter>();
    }
};

inline void init_logger(LogMode mode) {
    namespace fs = std::filesystem;
    fs::path log_dir = "logs";
    if (!fs::exists(log_dir)) {
        fs::create_directories(log_dir);
    }

    spdlog::init_thread_pool(8192, 1);

    std::vector<spdlog::sink_ptr> sinks;

    if (mode == LogMode::Dev) {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);
        sinks.push_back(console_sink);
    } else {
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/app.log", 10 * 1024 * 1024, 10);

        if (mode == LogMode::PreRelease) {
            rotating_sink->set_level(spdlog::level::debug);
        } else {
            rotating_sink->set_level(spdlog::level::info);
        }
        sinks.push_back(rotating_sink);
    }

    auto logger = std::make_shared<spdlog::async_logger>(
        "main_logger",
        sinks.begin(), sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

    spdlog::set_default_logger(logger);

    // 设置格式
    if (mode == LogMode::Dev) {
        // 彩色终端文本格式
        spdlog::set_pattern("%^[%Y%m%d.%H-%M-%S.%e] [%l] %v%$");
        spdlog::set_level(spdlog::level::debug);
    } else {
        // JSON 格式（需保证 unix_ms_formatter 已定义）
        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->add_flag<unix_ms_formatter>('u').set_pattern(R"({"time":"%u","level":"%l","msg":"%v"})");
        // 全局设置格式化器
        spdlog::set_formatter(std::move(formatter));
        spdlog::set_level(mode == LogMode::PreRelease ? spdlog::level::debug : spdlog::level::info);
    }

    spdlog::flush_on(spdlog::level::info);
}