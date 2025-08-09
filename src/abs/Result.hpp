#pragma once
#include <string>
#include <variant>
#include <stdexcept>
#include <utility>

template<typename T, typename E = std::string>
class Result {
public:
    using ValueType = T;
    using ErrorType = E;

    // 工厂方法
    static Result Ok(T value) {
        return Result(State::Ok, std::move(value), E{});
    }

    static Result Err(E error) {
        return Result(State::Err, T{}, std::move(error));
    }

    // 状态检查
    [[nodiscard]] bool isOk() const noexcept { return state_ == State::Ok; }
    [[nodiscard]] bool isErr() const noexcept { return state_ == State::Err; }

    // 访问 value
    T& value() {
        if (isErr()) throw std::logic_error("Tried to access value of an error Result");
        return *value_;
    }
    const T& value() const {
        if (isErr()) throw std::logic_error("Tried to access value of an error Result");
        return *value_;
    }

    // 访问 error
    E& error() {
        if (isOk()) throw std::logic_error("Tried to access error of an ok Result");
        return *error_;
    }
    const E& error() const {
        if (isOk()) throw std::logic_error("Tried to access error of an ok Result");
        return *error_;
    }

private:
    enum class State { Ok, Err };

    State state_;
    std::optional<T> value_;
    std::optional<E> error_;

    Result(State state, T value, E error)
        : state_(state) {
        if (state == State::Ok) value_.emplace(std::move(value));
        else error_.emplace(std::move(error));
    }
};

// ----------------------------------
// void 特化版本
// ----------------------------------
template<typename E>
class Result<void, E> {
public:
    using ValueType = void;
    using ErrorType = E;

    static Result Ok() {
        return Result(State::Ok, E{});
    }

    static Result Err(E error) {
        return Result(State::Err, std::move(error));
    }

    [[nodiscard]] bool isOk() const noexcept { return state_ == State::Ok; }
    [[nodiscard]] bool isErr() const noexcept { return state_ == State::Err; }

    const E& error() const {
        if (isOk()) throw std::logic_error("Tried to access error of an ok Result<void>");
        return error_;
    }

private:
    enum class State { Ok, Err };

    State state_;
    E error_;

    Result(State state, E error) : state_(state), error_(std::move(error)) {}
};
