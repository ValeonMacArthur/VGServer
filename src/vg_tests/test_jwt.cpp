//
// Created by mike on 25-8-9.
//

#include "../jwt-cpp/jwt.h"
#include "../jwt-cpp/jwt_auth.hpp"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <nlohmann/json.hpp>

TEST_CASE("JWT HS256 encode/decode works correctly", "[jwt]") {
    using namespace jwt;

    algorithm::hs256 alg("my_secret_key");

    json payload = {
        {"sub", "1234567890"},
        {"name", "John Doe"},
        {"admin", true}
    };

    std::string token = create(payload, alg);
    std::cout << "Generated JWT token: " << token << std::endl;
    REQUIRE_FALSE(token.empty());

    // 解码并验证签名成功
    json decoded_payload = decode(token, alg);
    REQUIRE(decoded_payload == payload);

    // 改变token任意字符后，解码时签名校验应该失败
    std::string bad_token = token;
    bad_token[10] = (bad_token[10] == 'a') ? 'b' : 'a'; // 简单改一字符

    REQUIRE_THROWS_AS(decode(bad_token, alg), std::runtime_error);

    // 关闭签名校验也能解析payload
    json decoded_no_verify = decode(bad_token, alg, false);
    REQUIRE(decoded_no_verify == payload);
}

TEST_CASE("HS256 algorithm sign and verify", "[jwt][algorithm]") {
    using namespace jwt;

    algorithm::hs256 alg("key123");

    std::string data = "data_to_sign";
    std::string signature = alg.sign(data);

    REQUIRE(alg.verify(data, signature));
    REQUIRE_FALSE(alg.verify(data, signature + "corrupt"));
}


using namespace auths;

struct TestClaims : public CustomClaims {
    TestClaims() {
        iss =App;
        aud = {Audience};
        iat = std::chrono::system_clock::now();
        exp = iat + RefreshTokenTTL;
    }
};

TEST_CASE("Generate and parse valid token") {
    // 构造 claims
    TestClaims claims;
    claims.sub = "user123";
    claims.metadata = {{"role", "admin"}};
    // 生成 token
    auto tokenRes = GenerateToken(claims, jwtAccessSecret);
    REQUIRE(tokenRes.isOk()); // 必须生成成功
    std::string token = tokenRes.value();
    REQUIRE_FALSE(token.empty()); // token 不应为空
    // 解析 token
    auto parseRes = ParseToken<TestClaims>(token, jwtAccessSecret);
    REQUIRE(parseRes.isOk()); // 必须解析成功
    const auto& parsed = parseRes.value();
    REQUIRE(parsed.sub == "user123");
    REQUIRE(parsed.metadata.at("role") == "admin");
    REQUIRE(parsed.iss == App);
    REQUIRE(parsed.aud.size() == 1);
    REQUIRE(parsed.aud[0] == Audience);
}