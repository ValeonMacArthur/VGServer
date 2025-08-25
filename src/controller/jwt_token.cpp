//
// Created by mike on 25-8-25.
//

#include "jwt_token.hpp"
// ========================
// TokenConfig 实现
// ========================
JWToken::TokenConfig& JWToken::TokenConfig::set_issuer(const std::string& value) {
    issuer = value;
    return *this;
}

JWToken::TokenConfig& JWToken::TokenConfig::set_expiry(std::chrono::seconds value) {
    expiry_duration = value;
    return *this;
}

JWToken::TokenConfig& JWToken::TokenConfig::add_claim(const std::string& key, const std::string& value) {
    additional_claims[key] = value;
    return *this;
}

// ========================
// JWToken 实现
// ========================
JWToken::JWToken(const std::string& secret_key, const std::string& service_name)
    : secret_(secret_key), service_(service_name) {}

const std::string& JWToken::secret() const noexcept { return secret_; }
void JWToken::set_secret(const std::string& value) { secret_ = value; }

const std::string& JWToken::service_name() const noexcept { return service_; }
void JWToken::set_service_name(const std::string& value) { service_ = value; }

std::string JWToken::encode(const TokenConfig& config) const {
    auto now = std::chrono::system_clock::now();

    auto token = jwt::create()
        .set_issuer(config.issuer)
        .set_type("JWT")
        .set_issued_at(now)
        .set_expires_at(now + config.expiry_duration);

    for (const auto& [key, value] : config.additional_claims) {
        token.set_payload_claim(key, jwt::claim(value));
    }

    return token.sign(jwt::algorithm::hs256{secret_});
}

std::optional<jwt::decoded_jwt<jwt::traits::kazuho_picojson>>
JWToken::decode(const std::string& token,
                const std::string& secret_key,
                const std::string& expected_issuer) {
    try {
        auto decoded = jwt::decode(token);

        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret_key})
            .with_issuer(expected_issuer);

        verifier.verify(decoded);
        return decoded;
    } catch (...) {
        return std::nullopt;
    }
}
