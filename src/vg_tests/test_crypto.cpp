//
// Created by III on 25-8-17.
//
#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include "../crypto/CryptoUtils.hpp"

using namespace CryptoUtils;

TEST_CASE("SHA-256 works", "[sha256]") {
    std::string msg = "abc";
    auto hash = sha256(msg);
    REQUIRE(hash == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST_CASE("AES encrypt/decrypt roundtrip", "[aes]") {
    std::string msg = "Hello AES!";
    ByteVec key(32, 'K'); // 256-bit
    ByteVec iv(16, 'I');  // 128-bit

    auto encrypted = aesEncrypt(msg, key, iv);
    auto decrypted = aesDecrypt(encrypted, key, iv);

    REQUIRE(decrypted == msg);
}

TEST_CASE("Base64 encode/decode roundtrip", "[base64]") {
    std::string msg = "Hello Base64!";
    ByteVec raw(msg.begin(), msg.end());

    auto encoded = base64Encode(raw);
    auto decoded = base64Decode(encoded);

    REQUIRE(std::string(decoded.begin(), decoded.end()) == msg);
}

TEST_CASE("RSA encrypt/decrypt roundtrip", "[rsa]") {
    std::string msg = "Hello RSA!";
    auto [pubKey, privKey] = generateRSAKeyPair();

    auto encrypted = rsaEncrypt(pubKey, msg);
    auto decrypted = rsaDecrypt(privKey, encrypted);

    REQUIRE(decrypted == msg);

    EVP_PKEY_free(pubKey);
    EVP_PKEY_free(privKey);
}