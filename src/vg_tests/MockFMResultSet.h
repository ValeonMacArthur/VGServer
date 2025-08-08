//
// Created by III on 2025/7/27.
//

#pragma once

#include "../abs/fm_resultset.hpp"
#include <optional>
#include <string>

class MockFMResultSet : public FMResultSet {
public:
    MockFMResultSet() : calledNext_(false) {}

    bool next() override {
        if (!calledNext_) {
            calledNext_ = true;
            return true;
        }
        return false;
    }

    Result<std::optional<int>> getInt(const std::string& columnName) override {
        if (columnName == "error") {
            return Result<std::optional<int>>::Err("Column error");
        }
        return Result<std::optional<int>>::Ok(123);
    }

    Result<std::optional<double>> getDouble(const std::string& columnName) override {
        if (columnName == "error") {
            return Result<std::optional<double>>::Err("Column error");
        }
        return Result<std::optional<double>>::Ok(45.6);
    }

    Result<std::optional<std::string>> getString(const std::string& columnName) override {
        if (columnName == "error") {
            return Result<std::optional<std::string>>::Err("Column error");
        }
        return Result<std::optional<std::string>>::Ok(std::string("test"));
    }

    Result<std::optional<int>> getInt(int columnIndex) override {
        if (columnIndex < 0 || columnIndex > 2) {
            return Result<std::optional<int>>::Err("Invalid column index");
        }
        return Result<std::optional<int>>::Ok(789);
    }

    Result<std::optional<double>> getDouble(int columnIndex) override {
        if (columnIndex < 0 || columnIndex > 2) {
            return Result<std::optional<double>>::Err("Invalid column index");
        }
        return Result<std::optional<double>>::Ok(12.34);
    }

    Result<std::optional<std::string>> getString(int columnIndex) override {
        if (columnIndex < 0 || columnIndex > 2) {
            return Result<std::optional<std::string>>::Err("Invalid column index");
        }
        return Result<std::optional<std::string>>::Ok(std::string("mock"));
    }

    Result<bool> isNull(const std::string& columnName) override {
        if (columnName == "null") {
            return Result<bool>::Ok(true);
        }
        if (columnName == "error") {
            return Result<bool>::Err("Column error");
        }
        return Result<bool>::Ok(false);
    }

    Result<bool> isNull(int columnIndex) override {
        if (columnIndex < 0 || columnIndex > 2) {
            return Result<bool>::Err("Invalid column index");
        }
        return (columnIndex == 2) ? Result<bool>::Ok(true) : Result<bool>::Ok(false);
    }

    int columnCount() const override {
        return 3;
    }

    std::string columnName(int index) const override {
        switch (index) {
            case 0: return "id";
            case 1: return "name";
            case 2: return "null";
            default: return "";
        }
    }

private:
    bool calledNext_;
};
