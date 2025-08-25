//
// Created by mike on 25-8-25.
//
#include <catch2/catch_test_macros.hpp>
#include "../controller/jwt_token.hpp"

TEST_CASE("JWToken basic encode/decode", "[jwt]") {
    using namespace std::chrono_literals;

    JWToken jwt("my-secret", "auth-service");

    JWToken::TokenConfig cfg;
    cfg.set_issuer("my-api")
       .set_expiry(10s)
       .add_claim("user_id", "42")
       .add_claim("role", "admin");

    std::string token = jwt.encode(cfg);

    SECTION("Decode valid token") {
        auto decoded = JWToken::decode(token, "my-secret", "my-api");
        REQUIRE(decoded.has_value());
        REQUIRE(decoded->get_issuer() == "my-api");
        REQUIRE(decoded->get_payload_claim("user_id").as_string() == "42");
        REQUIRE(decoded->get_payload_claim("role").as_string() == "admin");
    }

    SECTION("Decode with wrong secret should fail") {
        auto decoded = JWToken::decode(token, "wrong-secret", "my-api");
        REQUIRE_FALSE(decoded.has_value());
    }

    SECTION("Decode with wrong issuer should fail") {
        auto decoded = JWToken::decode(token, "my-secret", "wrong-api");
        REQUIRE_FALSE(decoded.has_value());
    }
}


TEST_CASE("JWToken getters and setters", "[jwt]") {
    JWToken jwt("init-secret", "init-service");

    REQUIRE(jwt.secret() == "init-secret");
    REQUIRE(jwt.service_name() == "init-service");

    jwt.set_secret("new-secret");
    jwt.set_service_name("new-service");

    REQUIRE(jwt.secret() == "new-secret");
    REQUIRE(jwt.service_name() == "new-service");
}