//
// Created by mike on 25-7-20.
//

#pragma once

#include "Result.hpp"
#include <optional>
#include <string>

class FMResultSet {
public:
    // 析构 & 禁拷贝支持移动
    virtual ~FMResultSet() = default;
    FMResultSet(const FMResultSet&) = delete;
    FMResultSet& operator=(const FMResultSet&) = delete;
    FMResultSet(FMResultSet&&) noexcept = default;
    FMResultSet& operator=(FMResultSet&&) noexcept = default;

    // 遍历
    [[nodiscard]] virtual bool next() = 0;

    // // 类型安全访问：按列名
    [[nodiscard]] virtual Result<std::optional<int>> getInt(const std::string& columnName) = 0;
    [[nodiscard]] virtual Result<std::optional<double>> getDouble(const std::string& columnName) = 0;
    [[nodiscard]] virtual Result<std::optional<std::string>> getString(const std::string& columnName) = 0;
    // 类型安全访问：按列下标
    [[nodiscard]] virtual Result<std::optional<int>> getInt(int columnIndex) = 0;
    [[nodiscard]] virtual Result<std::optional<double>> getDouble(int columnIndex) = 0;
    [[nodiscard]] virtual Result<std::optional<std::string>> getString(int columnIndex) = 0;
    // 判断是否为 NULL
    [[nodiscard]] virtual Result<bool> isNull(const std::string& columnName) = 0;
    [[nodiscard]] virtual Result<bool> isNull(int columnIndex) = 0;

    // 元信息
    [[nodiscard]] virtual int columnCount() const = 0;
    [[nodiscard]] virtual std::string columnName(int index) const = 0;

protected:
    FMResultSet() = default;
}; dms
