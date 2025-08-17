#pragma once
#include <string>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <vector>
#include <openssl/buffer.h>
namespace jwt {

    using json = nlohmann::json;

    namespace base64url {
        inline std::string encode(const std::string& in) {
            BIO* b64 = BIO_new(BIO_f_base64());
            BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
            BIO* mem = BIO_new(BIO_s_mem());
            b64 = BIO_push(b64, mem);
            BIO_write(b64, in.data(), static_cast<int>(in.size()));
            BIO_flush(b64);
            BUF_MEM* buffer_ptr;
            BIO_get_mem_ptr(b64, &buffer_ptr);
            std::string out(buffer_ptr->data, buffer_ptr->length);
            BIO_free_all(b64);
            for (auto& c : out) if (c == '+') c = '-'; else if (c == '/') c = '_';
            out.erase(std::remove(out.begin(), out.end(), '='), out.end());
            return out;
        }

        inline std::string decode(const std::string& in) {
            std::string tmp = in;
            for (auto& c : tmp) if (c == '-') c = '+'; else if (c == '_') c = '/';
            while (tmp.size() % 4) tmp += '=';
            BIO* b64 = BIO_new(BIO_f_base64());
            BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
            BIO* mem = BIO_new_mem_buf(tmp.data(), static_cast<int>(tmp.size()));
            mem = BIO_push(b64, mem);
            std::vector<char> out(tmp.size());
            int len = BIO_read(mem, out.data(), static_cast<int>(out.size()));
            BIO_free_all(mem);
            return std::string(out.data(), len);
        }
    }

    namespace algorithm {
        struct hs256 {
            explicit hs256(std::string key) : secret(std::move(key)) {}

            std::string sign(const std::string& data) const {
                unsigned char hash[EVP_MAX_MD_SIZE];
                unsigned int len = 0;
                HMAC(EVP_sha256(),
                     secret.data(), static_cast<int>(secret.size()),
                     reinterpret_cast<const unsigned char*>(data.data()),
                     static_cast<int>(data.size()),
                     hash, &len);
                return std::string(reinterpret_cast<char*>(hash), len);
            }

            bool verify(const std::string& data, const std::string& signature) const {
                return sign(data) == signature;
            }

            std::string secret;
        };
    }

    inline std::string create(const json& payload, const algorithm::hs256& alg) {
        json header = { {"alg", "HS256"}, {"typ", "JWT"} };
        std::string header_b64 = base64url::encode(header.dump());
        std::string payload_b64 = base64url::encode(payload.dump());
        std::string data = header_b64 + "." + payload_b64;
        std::string signature = base64url::encode(alg.sign(data));
        return data + "." + signature;
    }

    inline json decode(const std::string& token, const algorithm::hs256& alg, bool verify_sig = true) {
        auto first_dot = token.find('.');
        auto second_dot = token.find('.', first_dot + 1);
        if (first_dot == std::string::npos || second_dot == std::string::npos)
            throw std::runtime_error("Invalid token format");

        std::string header_b64 = token.substr(0, first_dot);
        std::string payload_b64 = token.substr(first_dot + 1, second_dot - first_dot - 1);
        std::string signature_b64 = token.substr(second_dot + 1);

        std::string header_json = base64url::decode(header_b64);
        std::string payload_json = base64url::decode(payload_b64);
        std::string signature = base64url::decode(signature_b64);

        if (verify_sig) {
            std::string data = header_b64 + "." + payload_b64;
            if (!alg.verify(data, signature))
                throw std::runtime_error("Invalid signature");
        }

        return json::parse(payload_json);
    }

    json decode(const std::string & token);
} // namespace jwt

