//
// Created by mike on 25-8-11.
//

#include "db_pool.hpp"
#include <iostream>

void DBPool::init_postgres(const std::string& host,
                           int port,
                           const std::string& dbname,
                           const std::string& user,
                           const std::string& password,
                           int initial_conns,
                           int max_conns,
                           int timeout_sec)
{
    if (pool_) {
        throw std::runtime_error("DBPool already initialized");
    }

    std::string url_str = "postgresql://" + user + ":" + password + "@" + host + ":" + std::to_string(port) + "/" + dbname + "?sslmode=disable";


    URL_T url = URL_new(url_str.c_str());
    if (!url) {
        throw std::runtime_error("Failed to parse database URL");
    }

    pool_ = ConnectionPool_new(url);
    if (!pool_) {
        URL_free(&url);
        throw std::runtime_error("Failed to create connection pool");
    }

    ConnectionPool_setInitialConnections(pool_, initial_conns);
    ConnectionPool_setMaxConnections(pool_, max_conns);
    ConnectionPool_setConnectionTimeout(pool_, timeout_sec);

    ConnectionPool_start(pool_);
    URL_free(&url);

    std::cout << "[DBPool] PostgreSQL connection pool initialized\n";
}

ConnectionPool_T DBPool::get_pool() {
    return pool_;
}

bool DBPool::test_connection() {
    if (!pool_) {
        std::cerr << "[DBPool] Pool not initialized\n";
        return false;
    }

    Connection_T con = ConnectionPool_getConnection(pool_);
    if (!con) {
        std::cerr << "[DBPool] Failed to get connection from pool\n";
        return false;
    }

    bool success = false;

    TRY {
        ResultSet_T rs = Connection_executeQuery(con, "SELECT 1");
        if (ResultSet_next(rs)) {
            success = true;
        }
        // 不用手动释放 rs
    } CATCH(SQLException) {
        std::cerr << "[DBPool] SQL error: " << Exception_frame.message << "\n";
    } END_TRY;

    Connection_close(con);

    return success;
}

void DBPool::shutdown() {
    if (pool_) {
        ConnectionPool_stop(pool_);
        ConnectionPool_free(&pool_);
        std::cout << "[DBPool] Connection pool shut down\n";
    }
}
