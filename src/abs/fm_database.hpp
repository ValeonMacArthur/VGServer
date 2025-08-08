//
// Created by mike on 25-7-20.
//

#pragma once

#include "fm_resultset.hpp"
#include "Result.hpp"
#include <memory>
#include <string>
#include <vector>

class FMDatabase {
public:
    virtual ~FMDatabase() = default;

    // 禁用拷贝构造和拷贝赋值
    FMDatabase(const FMDatabase&) = delete;
    FMDatabase& operator=(const FMDatabase&) = delete;

    // 支持移动
    FMDatabase(FMDatabase&&) noexcept = default;
    FMDatabase& operator=(FMDatabase&&) noexcept = default;

    // 生命周期
    virtual Result<void> open(const std::string& path) = 0;
    virtual Result<void> close() = 0;

    // 提供非虚包装
    Result<void> open() {
        return open(":memory:");
    }
    // 事务管理
    virtual Result<void> beginTransaction() = 0;
    virtual Result<void> commit() = 0;
    virtual Result<void> rollback() = 0;

    // SQL 执行：更新/插入/删除
    virtual Result<void> executeUpdate(const std::string& sql, const std::vector<std::string>& args) = 0;

    // SQL 执行：查询
    virtual Result<std::unique_ptr<FMResultSet>> executeQuery(const std::string& sql, const std::vector<std::string>& args) = 0;

    // 非虚包装函数（提供默认参数）
    Result<void> executeUpdate(const std::string& sql) {
        return executeUpdate(sql, {});
    }

    Result<std::unique_ptr<FMResultSet>> executeQuery(const std::string& sql) {
        return executeQuery(sql, {});
    }

protected:
    FMDatabase() = default;
};
