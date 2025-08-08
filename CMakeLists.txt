cmake_minimum_required(VERSION 3.22.1)
project(VGServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Catch2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(gflags REQUIRED)
find_package(folly REQUIRED)

add_executable(VGServer src/main.cpp
        src/abs/Result.hpp
        src/abs/fm_database.hpp
        src/abs/fm_resultset.hpp
        src/utils/log.hpp
        src/controller/router.hpp
        src/controller/controller.hpp
        src/controller/hello_controller.cpp
        src/controller/chat_ws_controller.cpp
        src/controller/chat_ws_controller.hpp
        src/controller/login_controller.cpp
)

target_include_directories(VGServer PRIVATE
        /usr/local/include/uWebSockets
        /usr/local/include/uSockets
)

# 链接 uSockets 静态库
target_link_libraries(VGServer PRIVATE
        spdlog::spdlog
        /usr/local/lib/uSockets.a
        -lz                 # zlib
        -lssl               # OpenSSL
        -lcrypto            # OpenSSL
        -lpthread
        gflags
        Folly::folly

)

# 编译测试程序
add_executable(vg_tests
        src/vg_tests/test_result.cpp
        src/vg_tests/test_fm.cpp
        src/vg_tests/MockFMResultSet.h
        src/model/model.hpp
        # 你可以加更多测试文件
)

# 测试程序链接 Catch2 的带 main 版本
target_link_libraries(vg_tests PRIVATE Catch2::Catch2WithMain)
enable_testing()
include(CTest)
include(Catch)
catch_discover_tests(vg_tests)