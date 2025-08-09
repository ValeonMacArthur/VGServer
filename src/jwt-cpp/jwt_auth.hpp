#pragma once
#include <string>
#include <map>
#include <chrono>
#include <vector>

#include "jwt.h"
#include "../abs/Result.hpp"

constexpr auto App = "vg_studio";
constexpr auto Audience = "com.apple.itunes";
constexpr std::chrono::hours AccessTokenTTL{1};       // 1小时
constexpr std::chrono::hours RefreshTokenTTL{24*7};   // 7天
inline const std::string jwtAccessSecret = "a3f9c1d4e5b67890f2a1b3c4d5e6f7890123456789abcdef0123456789abcdef";
inline const std::string jwtRefreshSecret = "f1e2d3c4b5a697887766554433221100abcdef1234567890fedcba0987654321";

namespace auths {
    using namespace jwt;
    using json = nlohmann::json;
    using namespace std::chrono_literals;

    struct CustomClaims {
        std::string sub;  // 用户ID
        std::map<std::string, std::string> metadata;
        std::string iss;  // issuer
        std::vector<std::string> aud;  // audience
        std::chrono::system_clock::time_point iat; // issued at
        std::chrono::system_clock::time_point exp; // expires at

        [[nodiscard]] Result<void, std::string> validate() const {
            auto now = std::chrono::system_clock::now();

            if (exp <= now) {
                return Result<void, std::string>::Err("token expired");
            }
            if (iss != App) {
                return Result<void, std::string>::Err("invalid issuer");
            }
            if (std::ranges::find(aud, Audience) == aud.end()) {
                return Result<void, std::string>::Err("invalid audience");
            }

            return Result<void, std::string>::Ok();
        }

        // 转 json 方便序列化
        [[nodiscard]] json to_json() const {
            json j;
            j["sub"] = sub;
            j["metadata"] = metadata;
            j["iss"] = iss;
            j["aud"] = aud;
            j["iat"] = std::chrono::duration_cast<std::chrono::seconds>(iat.time_since_epoch()).count();
            j["exp"] = std::chrono::duration_cast<std::chrono::seconds>(exp.time_since_epoch()).count();
            return j;
        }

        // 改成模板，方便支持派生类
        template<typename DerivedT = CustomClaims>
        static DerivedT from_json(const json& j) {
            DerivedT c;
            c.sub = j.at("sub").get<std::string>();
            c.metadata = j.value("metadata", std::map<std::string, std::string>{});
            c.iss = j.at("iss").get<std::string>();
            c.aud = j.at("aud").get<std::vector<std::string>>();
            c.iat = std::chrono::system_clock::time_point{std::chrono::seconds(j.at("iat").get<int64_t>())};
            c.exp = std::chrono::system_clock::time_point{std::chrono::seconds(j.at("exp").get<int64_t>())};
            return c;
        }
    };

    template<typename ClaimsT>
    Result<std::string, std::string> GenerateToken(const ClaimsT& claims, const std::string& key) {
        auto res = claims.validate();
        if (res.isErr()) {
            return Result<std::string, std::string>::Err(res.error());
        }
        std::string payload = claims.to_json().dump();
        jwt::algorithm::hs256 alg(key);
        std::string token = jwt::create(json::parse(payload), alg);
        return Result<std::string, std::string>::Ok(std::move(token));
    }

    template<typename ClaimsT>
    Result<ClaimsT, std::string> ParseToken(const std::string& token, const std::string& key) {
        try {
            jwt::algorithm::hs256 alg(key);
            json payload = jwt::decode(token, alg, true);

            // 调用模板化的 from_json
            ClaimsT claims = ClaimsT::template from_json<ClaimsT>(payload);
            auto res = claims.validate();
            if (res.isErr()) {
                return Result<ClaimsT, std::string>::Err(res.error());
            }

            return Result<ClaimsT, std::string>::Ok(std::move(claims));
        } catch (const std::exception& e) {
            return Result<ClaimsT, std::string>::Err(e.what());
        } catch (...) {
            return Result<ClaimsT, std::string>::Err("unknown error during token parsing");
        }
    }
}
