#pragma once
#include <jwt-cpp/jwt.h>
#include <string>
#include <chrono>
#include <map>
#include <optional>

class JWToken {
public:
    // ========================
    // TokenConfig - 配置结构体
    // ========================
    struct TokenConfig {
        std::string issuer{"my-api"};
        std::chrono::seconds expiry_duration{std::chrono::hours(24)};
        std::map<std::string, std::string> additional_claims{};

        // Builder 风格链式接口
        TokenConfig& set_issuer(const std::string& value);
        TokenConfig& set_expiry(std::chrono::seconds value);
        TokenConfig& add_claim(const std::string& key, const std::string& value);
    };

private:
    std::string secret_;
    std::string service_;

public:
    // 构造函数
    explicit JWToken(const std::string& secret_key, const std::string& service_name);

    // Getter / Setter
    [[nodiscard]] const std::string& secret() const noexcept;
    void set_secret(const std::string& value);

    [[nodiscard]] const std::string& service_name() const noexcept;

    void set_service_name(const std::string& value);

    // Encode
    [[nodiscard]] std::string encode(const TokenConfig& config) const;

    // Decode
    static std::optional<jwt::decoded_jwt<jwt::traits::kazuho_picojson>>
    decode(const std::string& token,
           const std::string& secret_key,
           const std::string& expected_issuer = "my-api");
};
