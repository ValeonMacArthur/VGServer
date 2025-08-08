//
// Created by III on 2025/7/27.
//
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <stdexcept>
#include <type_traits>
#include "../abs/Result.hpp"  // 你的 Result 模板类头文件

TEST_CASE("Result<int, std::string> basic usage") {
    auto r1 = Result<int, std::string>::Ok(42);
    REQUIRE(r1.isOk());
    REQUIRE_FALSE(r1.isErr());
    REQUIRE(r1.value() == 42);

    auto r2 = Result<int, std::string>::Err("error");
    REQUIRE(r2.isErr());
    REQUIRE_FALSE(r2.isOk());
    REQUIRE(r2.error() == "error");

    // 访问 value() 失败时抛异常
    REQUIRE_THROWS_AS(r2.value(), std::logic_error);
    // 访问 error() 失败时抛异常
    REQUIRE_THROWS_AS(r1.error(), std::logic_error);
}

TEST_CASE("Result<void, std::string> specialization usage") {
    auto r1 = Result<void, std::string>::Ok();
    REQUIRE(r1.isOk());
    REQUIRE_FALSE(r1.isErr());
    REQUIRE_THROWS_AS(r1.error(), std::logic_error);

    auto r2 = Result<void, std::string>::Err("failed");
    REQUIRE(r2.isErr());
    REQUIRE_FALSE(r2.isOk());
    REQUIRE(r2.error() == "failed");
}