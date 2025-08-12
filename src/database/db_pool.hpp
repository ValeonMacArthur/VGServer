//
// Created by mike on 25-8-11.
//

#pragma once

#include <string>
#include <stdexcept>

extern "C" {
#include <zdb/zdb.h>
}

class DBPool {
public:
    // 初始化 PostgreSQL 连接池
    static void init_postgres(const std::string& host,
                              int port,
                              const std::string& dbname,
                              const std::string& user,
                              const std::string& password,
                              int initial_conns = 2,
                              int max_conns = 10,
                              int timeout_sec = 5);

    // 获取连接池对象（可选）
    static ConnectionPool_T get_pool();

    // 测试连接是否正常，返回true表示成功
    static bool test_connection();

    // 关闭连接池释放资源
    static void shutdown();

private:
    static inline ConnectionPool_T pool_ = nullptr;
};