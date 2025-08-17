#pragma once
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <climits>   // for INT_MAX
namespace CryptoUtils {

using ByteVec = std::vector<unsigned char>;

// ---------- Base64 编码 ----------
inline std::string base64Encode(const ByteVec& data) {
    if (data.size() > static_cast<size_t>(INT_MAX)) {
        throw std::overflow_error("base64Encode input too large for BIO_write");
    }

    BUF_MEM* bufferPtr;

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bio = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bio);

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, data.data(), static_cast<int>(data.size()));  // ✅ safe cast
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bufferPtr);

    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(b64);
    return result;
}

// ---------- Base64 解码 ----------
inline ByteVec base64Decode(const std::string& input) {
    if (input.size() > static_cast<size_t>(INT_MAX)) {
        throw std::overflow_error("base64Decode input too large for BIO_new_mem_buf");
    }

    int len = static_cast<int>(input.size());  // ✅ safe cast
    ByteVec buffer(len);

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bio = BIO_new_mem_buf(input.data(), len);
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    int decodedLen = BIO_read(bio, buffer.data(), len);
    if (decodedLen < 0) throw std::runtime_error("BIO_read failed");
    buffer.resize(decodedLen);

    BIO_free_all(bio);
    return buffer;
}

// ---------- SHA-256 ----------
inline std::string sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.data()), input.size(), hash);

    std::string output;
    char buf[3];
    for (const unsigned char c : hash) {
        snprintf(buf, sizeof(buf), "%02x", c);
        output.append(buf);
    }
    return output;
}

// ---------- AES 加密 ----------
inline ByteVec aesEncrypt(const std::string& plaintext, const ByteVec& key, const ByteVec& iv) {
    if (plaintext.size() > static_cast<size_t>(INT_MAX)) {
        throw std::overflow_error("aesEncrypt input too large for EVP_EncryptUpdate");
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1)
        throw std::runtime_error("EVP_EncryptInit_ex failed");

    ByteVec ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);
    int len = 0, ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                          reinterpret_cast<const unsigned char*>(plaintext.data()),
                          static_cast<int>(plaintext.size())) != 1)   // ✅ safe cast
        throw std::runtime_error("EVP_EncryptUpdate failed");
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1)
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    ciphertext_len += len;

    ciphertext.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

// ---------- AES 解密 ----------
inline std::string aesDecrypt(const ByteVec& ciphertext, const ByteVec& key, const ByteVec& iv) {
    if (ciphertext.size() > static_cast<size_t>(INT_MAX)) {
        throw std::overflow_error("aesDecrypt input too large for EVP_DecryptUpdate");
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1)
        throw std::runtime_error("EVP_DecryptInit_ex failed");

    ByteVec plaintext(ciphertext.size());
    int len = 0, plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(),
                          static_cast<int>(ciphertext.size())) != 1)   // ✅ safe cast
        throw std::runtime_error("EVP_DecryptUpdate failed");
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1)
        throw std::runtime_error("EVP_DecryptFinal_ex failed");
    plaintext_len += len;

    plaintext.resize(plaintext_len);
    EVP_CIPHER_CTX_free(ctx);
    return {reinterpret_cast<char*>(plaintext.data()), plaintext.size()};
}

// ---------- RSA 密钥生成 ----------
inline std::pair<EVP_PKEY*, EVP_PKEY*> generateRSAKeyPair(int bits = 2048) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) throw std::runtime_error("EVP_PKEY_CTX_new_id failed");

    if (EVP_PKEY_keygen_init(ctx) <= 0) throw std::runtime_error("EVP_PKEY_keygen_init failed");
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0)
        throw std::runtime_error("EVP_PKEY_CTX_set_rsa_keygen_bits failed");

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) throw std::runtime_error("EVP_PKEY_keygen failed");

    EVP_PKEY_CTX_free(ctx);
    return { pkey, EVP_PKEY_dup(pkey) };
}

// ---------- RSA 加密 ----------
inline ByteVec rsaEncrypt(EVP_PKEY* pubKey, const std::string& message) {
    if (message.size() > static_cast<size_t>(INT_MAX)) {
        throw std::overflow_error("rsaEncrypt input too large for EVP_PKEY_encrypt");
    }

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pubKey, nullptr);
    if (!ctx) throw std::runtime_error("EVP_PKEY_CTX_new failed");

    if (EVP_PKEY_encrypt_init(ctx) <= 0) throw std::runtime_error("EVP_PKEY_encrypt_init failed");

    size_t outlen;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen,
                         reinterpret_cast<const unsigned char*>(message.data()), message.size()) <= 0)
        throw std::runtime_error("EVP_PKEY_encrypt size failed");

    ByteVec out(outlen);
    if (EVP_PKEY_encrypt(ctx, out.data(), &outlen,
                         reinterpret_cast<const unsigned char*>(message.data()), message.size()) <= 0)
        throw std::runtime_error("EVP_PKEY_encrypt failed");

    out.resize(outlen);
    EVP_PKEY_CTX_free(ctx);
    return out;
}

// ---------- RSA 解密 ----------
inline std::string rsaDecrypt(EVP_PKEY* pKey, const ByteVec& ciphertext) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pKey, nullptr);
    if (!ctx) throw std::runtime_error("EVP_PKEY_CTX_new failed");

    if (EVP_PKEY_decrypt_init(ctx) <= 0) throw std::runtime_error("EVP_PKEY_decrypt_init failed");

    size_t outlen;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, ciphertext.data(), ciphertext.size()) <= 0)
        throw std::runtime_error("EVP_PKEY_decrypt size failed");

    ByteVec out(outlen);
    if (EVP_PKEY_decrypt(ctx, out.data(), &outlen, ciphertext.data(), ciphertext.size()) <= 0)
        throw std::runtime_error("EVP_PKEY_decrypt failed");

    out.resize(outlen);
    EVP_PKEY_CTX_free(ctx);
    return {reinterpret_cast<char*>(out.data()), out.size()};
}

} // namespace CryptoUtils
