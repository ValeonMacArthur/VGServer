cmake_minimum_required(VERSION 3.22.1)
project(VGServer)
cmake_policy(SET CMP0167 OLD)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_package(Catch2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)



add_executable(VGServer src/main.cpp
        src/abs/Result.hpp
        src/abs/fm_database.hpp
        src/abs/fm_resultset.hpp
        src/utils/log.hpp
        src/database/db_pool.cpp
        src/database/db_pool.hpp
        src/gin/UWSWrapper.hpp
        src/controller/hello.cpp
        src/controller/hello.hpp
        src/controller/pub.hpp
        src/crypto/CryptoUtils.hpp
        src/controller/aes.cpp
        src/controller/aes.hpp
        src/controller/jwt_token.cpp
        src/controller/jwt_token.hpp
        src/gin/uWSGuard.hpp
)

target_include_directories(VGServer PRIVATE
        /usr/local/include
        /usr/local/include/uWebSockets
        /usr/local/include/uSockets
        /usr/local/include/zdb
        /usr/local/include/nlohmann
        /usr/local/include/boost/pool
)

# 链接 uSockets 静态库
target_link_libraries(VGServer PRIVATE
        spdlog::spdlog
        /usr/local/lib/uSockets.a
        ZLIB::ZLIB
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
        -lzdb
)

# 编译测试程序
add_executable(vg_tests
        src/vg_tests/test_result.cpp
        src/vg_tests/test_fm.cpp
        src/vg_tests/MockFMResultSet.h
        src/database/db_pool.cpp
        src/database/db_pool.hpp
        src/vg_tests/concurrentqueue.cpp
        src/crypto/CryptoUtils.hpp
        src/vg_tests/test_crypto.cpp
        src/controller/jwt_token.cpp
        src/controller/jwt_token.hpp
        src/vg_tests/test_jwt.cpp
        # 你可以加更多测试文件
)
target_include_directories(vg_tests PRIVATE
        /usr/local/include/catch2
        /usr/local/include/zdb
        /usr/local/include/nlohmann
        /usr/local/include
)
# 测试程序链接 Catch2 的带 main 版本
target_link_libraries(vg_tests PRIVATE
        Catch2::Catch2WithMain
        OpenSSL::SSL
        OpenSSL::Crypto
        -lzdb
        )
enable_testing()
include(CTest)
include(Catch)
catch_discover_tests(vg_tests)